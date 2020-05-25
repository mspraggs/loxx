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

#include <numeric>
#include <utility>

#include "../HashTable.hpp"

#include "logging.hpp"
#include "RegisterAllocator.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    std::size_t count_snapshot_mappings(const std::vector<Snapshot>& snapshots)
    {
      std::size_t ret = 0;
      std::accumulate(
          snapshots.begin(), snapshots.end(), 0ul,
          [] (const std::size_t value, const Snapshot& snapshot) {
            return value + snapshot.stack_ir_map.size();
          });
      return ret;
    }


    std::vector<Range> compute_live_ranges(
        const IRBuffer<2>& ir_buffer, const std::vector<Snapshot>& snapshots)
    {
      std::vector<bool> ref_seen(ir_buffer.size(), false);
      std::vector<Range> live_ranges;
      live_ranges.reserve(ir_buffer.size());
      auto snap_it = snapshots.rbegin();

      for (int i = ir_buffer.size() - 1; i >= 0; --i) {
        const auto& instruction = ir_buffer[i];

        const auto& op0 = instruction.operand(0);
        const auto& op1 = instruction.operand(1);

        if (op0.type() == Operand::Type::IR_REF) {
          const auto ref = unsafe_get<std::size_t>(op0);
          if (not ref_seen[ref]) {
            live_ranges.emplace_back(ref, i);
            ref_seen[ref] = true;
          }
        }

        if (op1.type() == Operand::Type::IR_REF) {
          const auto ref = unsafe_get<std::size_t>(op1);
          if (not ref_seen[ref]) {
            live_ranges.emplace_back(ref, i);
            ref_seen[ref] = true;
          }
        }
      }

      std::sort(
          live_ranges.begin(), live_ranges.end(),
          [] (const Range& first, const Range& second) {
            return first.first < second.first;
          });

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
      const auto live_ranges =
          compute_live_ranges(trace.ir_buffer, trace.snaps);
      const auto& ir_buffer = trace.ir_buffer;

      for (const auto& live_range : live_ranges) {
        expire_old_intervals(live_range);

        const auto& ir_instruction = ir_buffer[live_range.first];
        const auto candidate_register = get_register(ir_instruction.type());

        if (not candidate_register) {
          spill_at_interval(live_range);
        }
        else {
          trace.allocation_map[live_range.first] = *candidate_register;
          insert_active_interval(live_range);
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


    Optional<Register> RegisterAllocator::get_register(const ValueType type)
    {
      for (unsigned int i = 0; i < register_pool_.size(); ++i) {
        if (register_pool_[i] and
            reg_supports_value_type(static_cast<Register>(i), type)) {
          register_pool_[i] = false;
          return static_cast<Register>(i);
        }
      }

      return {};
    }


    void RegisterAllocator::add_register(const Register reg)
    {
      register_pool_[static_cast<unsigned int>(reg)] = true;
    }
  }
}
