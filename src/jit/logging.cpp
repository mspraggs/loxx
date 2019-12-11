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
 * Created by Matt Spraggs on 15/11/2019.
 */

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "logging.hpp"
#include "SSAInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    void print_ssa_instruction(const SSAInstruction<2>& instruction)
    {
      std::cout << std::setw(20) << std::setfill(' ') << instruction.op();

      const auto& operands = instruction.operands();

      for (std::size_t i = 0; i < operands.size(); ++i) {
        if (i != 0) {
          std::cout << ", ";
        }
        if (not operands[i].is_used()) {
          break;
        }
        std::cout << operands[i];
      }

      std::cout << '\n';
    }


    void print_ssa_instructions(
        const std::vector<SSAInstruction<2>>& instructions)
    {
        for (std::size_t i = 0; i < instructions.size(); ++i) {
          std::cout << std::setw(4) << std::setfill('0') << std::right
                    << i << ' ' << std::left;
          print_ssa_instruction(instructions[i]);
        }
    }


    void print_live_ranges(
        const std::vector<std::pair<Operand, Range>>& live_ranges)
    {
      using OperandRangePair = std::pair<Operand, Range>;

      const auto range_max_elem = std::max_element(
          live_ranges.begin(), live_ranges.end(),
          [] (const OperandRangePair& first, const OperandRangePair& second) {
            return first.second.second < second.second.second;
          }
      );
      const auto range_max = range_max_elem->second.second;

      std::cout << "      ";
      for (std::size_t i = 0; i < range_max + 1; ++i) {
        std::cout << i / 10;
      }
      std::cout << '\n';

      std::cout << "      ";
      for (std::size_t i = 0; i < range_max + 1; ++i) {
        std::cout << i % 10;
      }
      std::cout << '\n';

      for (const auto& live_range : live_ranges) {
        const auto begin = live_range.second.first;
        const auto end = live_range.second.second;

        std::stringstream ss;
        ss << live_range.first;

        std::cout << std::left << std::setw(6) << ss.str();
        for (std::size_t i = 0; i < end + 1; ++i) {
          if (i < live_range.second.first) {
            std::cout << ' ';
          }
          else if (i == end) {
            std::cout << (begin < end ? '>' : 'v');
          }
          else {
            std::cout << '-';
          }
        }
        std::cout << '\n';
      }
    }


    void print_allocation_map(const AllocationMap& allocation_map)
    {
      for (const auto& allocation : allocation_map) {
        const auto is_register = holds_alternative<Register>(allocation.second);
        std::cout << allocation.first << " -> ";
        if (is_register) {
          std::cout << unsafe_get<Register>(allocation.second) << "\n";
        }
        else {
          std::cout << "[ +" << std::right << std::setw(3) << std::setfill('0')
                    << unsafe_get<std::size_t>(allocation.second) << " ]\n";
        }
      }
    }
  }
}
