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
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    std::vector<std::pair<std::size_t, Range>> compute_live_ranges(
        const IRBuffer<2>& ir_buffer, const std::vector<Snapshot>& snapshots)
    {
      std::vector<Optional<std::size_t>> operand_start_map(ir_buffer.size());
      std::vector<std::pair<std::size_t, Range>> live_ranges;
      live_ranges.reserve(ir_buffer.size() * 2);
      auto snap_it = snapshots.begin();

      const auto update_live_ranges = [&] (
          const std::size_t ref0, const std::size_t ref1) {
        const auto new_entry = not static_cast<bool>(operand_start_map[ref0]);

        if (new_entry) {
          operand_start_map[ref0] = live_ranges.size();
          live_ranges.push_back({ref0, Range{ref1, ref1}});
        }

        auto& live_range = live_ranges[*operand_start_map[ref0]];
        live_range.second.second = ref1;
      };

      for (std::size_t i = 0; i < ir_buffer.size(); ++i) {
        const auto& instruction = ir_buffer[i];

        if (instruction.type() != ValueType::UNKNOWN) {
          update_live_ranges(i, i);
        }

        for (const auto& operand : instruction.operands()) {
          if (operand.type() == Operand::Type::UNUSED) {
            break;
          }

          if (operand.type() != Operand::Type::IR_REF) {
            continue;
          }

          const auto reg = unsafe_get<std::size_t>(operand);
          update_live_ranges(reg, i);
        }

        while (i == snap_it->ir_ref) {
          for (const auto& mapping : snap_it->stack_ir_map) {
            update_live_ranges(mapping.second.value, i);
          }
          ++snap_it;
        }
      }

      return live_ranges;
    }


    RegisterAllocator::RegisterAllocator(
        const bool debug, const std::vector<Register>& registers)
        : debug_(debug), stack_index_(0)
    {
      const auto max_reg = std::max_element(
          registers.begin(), registers.end(),
          [] (const Register first, const Register second) {
            return static_cast<unsigned int>(first) <
                static_cast<unsigned int>(second);
          });

      register_pool_.resize(static_cast<unsigned int>(*max_reg) + 1, false);
      for (const auto reg : registers) {
        add_register(reg);
      }
    }


    void RegisterAllocator::allocate(Trace& trace)
    {
      trace_ = &trace;
      trace.allocation_map.resize(trace.ir_buffer.size(), {});
      auto live_ranges = compute_live_ranges(trace.ir_buffer, trace.snaps);
      const auto& ir_buffer = trace.ir_buffer;

      for (const auto& live_range : live_ranges) {
        const auto& virtual_register = live_range.first;
        const auto& interval = live_range.second;

        if (interval.first == interval.second) {
          continue;
        }

        expire_old_intervals(interval);

        const auto& ir_instruction = ir_buffer[virtual_register];
        const auto candidate_register = get_register(ir_instruction.type());

        if (not candidate_register) {
          spill_at_interval(interval);
        }
        else {
          trace.allocation_map[interval.first] = *candidate_register;
          insert_active_interval(interval);
        }
      }
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
        const auto& reg = trace_->allocation_map[active_interval.first];
        add_register(get<Register>(reg.value()));
      }

      for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
        active_intervals_.erase(*it);
      }
    }


    void RegisterAllocator::spill_at_interval(const Range& interval)
    {
      const auto& spill_interval = active_intervals_.back();

      if (spill_interval.second > interval.second) {
        const auto elem = trace_->allocation_map[spill_interval.first];
        trace_->allocation_map[interval.first] = get<Register>(elem.value());
        trace_->allocation_map[spill_interval.first] = stack_index_++;

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
      for (unsigned int i = 0; i < register_pool_.size(); ++i) {
        if (register_pool_[i] and
            reg_supports_value_type(static_cast<Register>(i), type)) {
          register_pool_[i] = false;
          return static_cast<Register>(i);
        }
      }
    }


    void RegisterAllocator::add_register(const Register reg)
    {
      register_pool_[static_cast<unsigned int>(reg)] = true;
    }
  }
}
