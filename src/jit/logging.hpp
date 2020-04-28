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

#ifndef LOXX_JIT_LOGGING_HPP
#define LOXX_JIT_LOGGING_HPP

#include <iomanip>
#include <iostream>
#include <sstream>

#include "RegisterAllocator.hpp"
#include "IRInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    template <std::size_t N>
    void print_ssa_instruction(const IRInstruction<N>& instruction);

    template <std::size_t N>
    void print_ssa_instructions(
        const std::vector<IRInstruction<N>>& instructions);

    void print_live_ranges(
        const std::vector<std::pair<std::size_t, Range>>& live_ranges);

    void print_allocation_map(const AllocationMap<Register>& allocation_map);

    template <std::size_t N>
    void print_ssa_instruction(const IRInstruction<N>& instruction)
    {
      std::cout << std::setw(20) << std::setfill(' ') << instruction.op();
      std::cout << std::setw(10) << std::setfill(' ') << instruction.type();

      const auto& operands = instruction.operands();

      for (std::size_t i = 0; i < operands.size(); ++i) {
        if (operands[i].type() == Operand::Type::UNUSED) {
          break;
        }
        if (i != 0) {
          // std::cout << ", ";
        }
        std::stringstream ss;
        ss << operands[i];
        std::cout << std::setw(15) << ss.str();
      }

      std::cout << '\n';
    }

    template <std::size_t N>
    void print_ssa_instructions(
        const std::vector<IRInstruction<N>>& instructions)
    {
        for (std::size_t i = 0; i < instructions.size(); ++i) {
          std::cout << std::setw(4) << std::setfill('0') << std::right
                    << i << ' ' << std::left;
          print_ssa_instruction(instructions[i]);
        }
    }
  }
}

#endif // LOXX_JIT_LOGGING_HPP
