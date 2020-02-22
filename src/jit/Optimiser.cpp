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
 * Created by Matt Spraggs on 21/02/2020.
 */

#include "Optimiser.hpp"


namespace loxx
{
  namespace jit
  {
    void optimise(SSABuffer<3>& ssa_ir)
    {
      for (std::size_t i = 0; i < ssa_ir.size() - 1; ++i) {
        const auto& current_instruction = ssa_ir[i];
        const auto& next_instruction = ssa_ir[i + 1];

        if (current_instruction.op() == Operator::MOVE and
            next_instruction.op() == Operator::MOVE and
            current_instruction.operands()[0] == next_instruction.operands()[0]) {
          ssa_ir[i] = SSAInstruction<3>(Operator::NOOP);
        }
      }
    }
  }
}
