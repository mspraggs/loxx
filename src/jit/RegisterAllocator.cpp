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


    void RegisterAllocator::allocate(
        const std::vector<SSAInstruction<2>>& ssa_ir)
    {
      auto live_ranges = compute_live_ranges(ssa_ir);

#ifndef NDEBUG
      if (debug_) {
        std::cout << "=== Register live ranges ===\n";
        print_live_ranges(live_ranges);
      }
#endif
    }


    std::size_t detail::combine_hashes(
        const std::size_t first, const std::size_t second)
    {
      return first + 0x9e3779b9 + (second << 6) + (second >> 2);
    }
  }
}
