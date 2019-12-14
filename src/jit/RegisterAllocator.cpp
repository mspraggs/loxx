/*
 * This file is part of loxx.
 *
 * loxx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by Matt Spraggs on 23/11/2019.
 */

#include <iostream>
#include <utility>

#include "../HashTable.hpp"

#include "logging.hpp"
#include "RegisterAllocator.hpp"


namespace loxx
{
  namespace jit
  {
    namespace detail
    {
      std::size_t combine_hashes(
          const std::size_t first, const std::size_t second);
    }


    std::vector<std::pair<Operand, Range>> compute_live_ranges(
        const std::vector<SSAInstruction<2>>& ssa_ir)
    {
      using OperandSizeMap =
          HashTable<Operand, std::size_t, OperandHasher, OperandCompare>;

      OperandSizeMap operand_start_map;
      std::vector<std::pair<Operand, Range>> live_ranges;
      live_ranges.reserve(ssa_ir.size() * 2);

      for (std::size_t i = 0; i < ssa_ir.size(); ++i) {
        const auto& instruction = ssa_ir[i];

        for (const auto& operand : instruction.operands()) {
          if (not operand.is_used()) {
            break;
          }

          if (not operand.is_register()) {
            continue;
          }

          auto operand_start =
              operand_start_map.insert(operand, live_ranges.size());

          if (operand_start.second) {
            live_ranges.push_back({operand, Range{i, i}});
          }

          auto& live_range = live_ranges[operand_start.first->second];
          live_range.second.second = i;
        }
      }

      return live_ranges;
    }


    RegisterAllocator::RegisterAllocator(
        const bool debug, const std::vector<Register>& registers)
        : debug_(debug), stack_index_(0)
    {
      for (const auto reg : registers) {
        register_pool_.insert(reg);
      }
    }


    AllocationMap RegisterAllocator::allocate(
        const std::vector<SSAInstruction<2>>& ssa_ir)
    {
      auto live_ranges = compute_live_ranges(ssa_ir);

#ifndef NDEBUG
      if (debug_) {
        std::cout << "=== Register live ranges ===\n";
        print_live_ranges(live_ranges);
      }
#endif

      for (const auto& live_range : live_ranges) {
        const auto& virtual_regsiter = live_range.first;
        const auto& interval = live_range.second;

        if (interval.first == interval.second) {
          continue;
        }

        expire_old_intervals(interval);

        const auto candidate_register = get_register(
            virtual_regsiter.value_type());

        if (not candidate_register) {
          spill_at_interval(interval);
        }
        else {
          registers_[interval.first] = *candidate_register;
          insert_active_interval(interval);
        }
      }

      AllocationMap allocation_map;

      for (const auto& live_range : live_ranges) {
        const auto& virtual_register = live_range.first;
        const auto& interval = live_range.second;

        if (interval.first == interval.second) {
          continue;
        }

        const auto& reg = registers_.get(interval.first);
        if (reg) {
          allocation_map[virtual_register] = Allocation(
              InPlace<Register>(), reg->second);
        }
        else {
          allocation_map[virtual_register] = Allocation(
              InPlace<std::size_t>(), stack_slots_[interval.first]);
        }
      }

      return allocation_map;
    }


    void RegisterAllocator::expire_old_intervals(const Range& interval)
    {
      std::vector<decltype(active_intervals_.begin())> to_remove;
      to_remove.reserve(active_intervals_.size());

      for (
          auto it = active_intervals_.begin();
          it != active_intervals_.end(); ++it) {

        const auto active_interval = *it;

        if (active_interval.second >= interval.first) {
          break;
        }

        to_remove.push_back(it);
        const auto& reg = registers_[active_interval.first];
        register_pool_.insert(reg);
      }

      for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
        active_intervals_.erase(*it);
      }
    }


    void RegisterAllocator::spill_at_interval(const Range& interval)
    {
      const auto& spill_interval = active_intervals_.back();

      if (spill_interval.second > interval.second) {
        const auto elem = registers_.find(spill_interval.first);
        registers_[interval.first] = elem->second;
        registers_.erase(elem);
        stack_slots_[spill_interval.first] = stack_index_++;

        remove_active_interval(spill_interval);
        insert_active_interval(interval);
      }
      else {
        stack_slots_[interval.first] = stack_index_++;
      }
    }


    void RegisterAllocator::insert_active_interval(const Range& interval)
    {
      const auto new_pos = std::upper_bound(
          active_intervals_.begin(), active_intervals_.end(), interval,
          [] (const Range& first, const Range second) {
            return first.second < second.second;
          });

      active_intervals_.insert(new_pos, interval);
    }


    void RegisterAllocator::remove_active_interval(const Range& interval)
    {
      const auto pos = std::lower_bound(
          active_intervals_.begin(), active_intervals_.end(), interval,
          [] (const Range& first, const Range second) {
            return first.second < second.second;
          });

      active_intervals_.erase(pos);
    }


    Optional<Register> RegisterAllocator::get_register(const ValueType type)
    {
      const auto elem = std::find_if(
        register_pool_.begin(), register_pool_.end(),
        [=] (const Register& reg) {
          return reg_supports_value_type(reg, type);
        }
      );
      if (elem == register_pool_.end()) {
        return {};
      }
      const auto ret = *elem;
      register_pool_.erase(elem);
      return ret;
    }


    std::size_t OperandHasher::operator() (const Operand& value) const
    {
      const auto op_type = static_cast<std::size_t>(value.op_type());
      const auto value_type = static_cast<std::size_t>(value.value_type());
      const auto content_hash = [&] {
        if (value.is_memory()) {
          return pointer_hasher(value.memory_address());
        }
        return value.reg_index();
      } ();

      return detail::combine_hashes(
          detail::combine_hashes(op_type, value_type), content_hash);
    }


    bool OperandCompare::operator() (
        const Operand& first, const Operand& second) const
    {
      if (first.op_type() != second.op_type() or
          first.value_type() != second.value_type()) {
        return false;
      }
      if (first.is_memory()) {
        return first.memory_address() == second.memory_address();
      }
      if (first.is_register()) {
        return first.reg_index() == second.reg_index();
      }
      return true;
    }


    std::size_t detail::combine_hashes(
        const std::size_t first, const std::size_t second)
    {
      return first + 0x9e3779b9 + (second << 6) + (second >> 2);
    }
  }
}
