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
    std::vector<std::pair<VirtualRegister, Range>> compute_live_ranges(
        const SSABuffer<3>& ssa_ir)
    {
      using OperandSizeMap = VRegHashTable<std::size_t>;

      OperandSizeMap operand_start_map;
      std::vector<std::pair<VirtualRegister, Range>> live_ranges;
      live_ranges.reserve(ssa_ir.size() * 2);

      for (std::size_t i = 0; i < ssa_ir.size(); ++i) {
        const auto& instruction = ssa_ir[i];

        for (const auto& operand : instruction.operands()) {
          if (operand.index() == Operand::npos) {
            break;
          }

          if (not holds_alternative<VirtualRegister>(operand)) {
            continue;
          }

          const auto& reg = unsafe_get<VirtualRegister>(operand);

          auto operand_start =
              operand_start_map.insert(reg, live_ranges.size());

          if (operand_start.second) {
            live_ranges.push_back({reg, Range{i, i}});
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


    AllocationMap<Register> RegisterAllocator::allocate(
        const SSABuffer<3>& ssa_ir)
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

        const auto candidate_register = get_register(virtual_regsiter.type);

        if (not candidate_register) {
          spill_at_interval(interval);
        }
        else {
          registers_[interval.first] = *candidate_register;
          insert_active_interval(interval);
        }
      }

      AllocationMap<Register> allocation_map;

      for (const auto& live_range : live_ranges) {
        const auto& virtual_register = live_range.first;
        const auto& interval = live_range.second;

        if (interval.first == interval.second) {
          continue;
        }

        const auto& reg = registers_.get(interval.first);
        if (reg) {
          allocation_map[virtual_register] = Allocation<Register>(
              InPlace<Register>(), reg->second);
        }
        else {
          allocation_map[virtual_register] = Allocation<Register>(
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


    Optional<Register> RegisterAllocator::get_register(
        const ValueType type)
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
  }
}
