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
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    void print_trace(const Trace& trace);

    template <std::size_t N>
    void print_ssa_instruction(const IRInstruction<N>& instruction);

    template <std::size_t N>
    void print_ssa_instructions(
        const std::vector<IRInstruction<N>>& instructions);

    void print_live_ranges(
        const std::vector<std::pair<std::size_t, Range>>& live_ranges);

    void print_allocation_map(const AllocationMap<Register>& allocation_map);

    template <std::size_t N>
    void print_ssa_instruction(
        const Trace& trace, const IRInstruction<N>& instruction)
    {
      std::cout << std::setw(15) << std::setfill(' ') << instruction.op();
      std::cout << std::setw(10) << std::setfill(' ') << instruction.type();

      const auto& operands = instruction.operands();

      for (std::size_t i = 0; i < operands.size(); ++i) {
        if (operands[i].type() == Operand::Type::UNUSED) {
          break;
        }
        std::stringstream ss;
        if (operands[i].type() == Operand::Type::LITERAL) {
          const auto& value = trace.code_object->constants[operands[i].index()];
          ss << std::setw(5) << std::setfill('.') << std::left;
          ss << static_cast<ValueType>(value.index()) << "[ '";
          if (holds_alternative<double>(value)) {
            ss << unsafe_get<double>(value);
          }
          else if (holds_alternative<bool>(value)) {
            ss << unsafe_get<bool>(value);
          }
          else if (holds_alternative<ObjectPtr>(value)) {
            ss << std::hex << unsafe_get<ObjectPtr>(value);
          }
          else {
            ss << "nil";
          }
          ss << "' ]";
        }
        else {
          ss << operands[i];
        }
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

    void print_snapshot(const std::size_t snap_num, const Snapshot& snapshot);
  }
}

#endif // LOXX_JIT_LOGGING_HPP
