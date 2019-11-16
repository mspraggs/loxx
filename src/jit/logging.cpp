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

#include <cassert>
#include <iomanip>
#include <iostream>

#include "SSAInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    void print_ssa_instruction(const SSAInstruction& instruction)
    {
      std::cout << std::setw(20) << std::setfill(' ') << instruction.op();

      const auto& operand0 = instruction.operand0();
      const auto& operand1 = instruction.operand1();

      if (operand0.is_used()) {
        std::cout << operand0;
      }

      if (operand1.is_used()) {
        std::cout << ", " << operand1;
      }

      std::cout << '\n';
    }
  }
}
