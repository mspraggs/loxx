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
    bool instructions_target_same_dest(
        const IRInstruction<3>& first, const IRInstruction<3>& second)
    {
      return first.operands()[0] == second.operands()[0];
    }

    bool instructions_contain_redundant_move(
        const IRInstruction<3>& first, const IRInstruction<3>& second)
    {
      return second.op() == Operator::MOVE and
          first.operands()[0] == second.operands()[1];
    }

    void optimise(IRBuffer<3>& ssa_ir)
    {
      for (std::size_t i = 1; i < ssa_ir.size(); ++i) {
        const auto& prev_instruction = ssa_ir[i - 1];
        const auto& current_instruction = ssa_ir[i];
        const auto& current_operands = current_instruction.operands();

        for (std::size_t j = 1; j < 4; ++j) {
          if (i >= j) {
            const auto& prior_instruction = ssa_ir[i - j];

            if (instructions_target_same_dest(
                prior_instruction, current_instruction)) {
              ssa_ir[i - j] = IRInstruction<3>(
                  Operator::NOOP, ValueType::UNKNOWN);
            }
          }
        }

        if (instructions_contain_redundant_move(
            prev_instruction, current_instruction)) {
          ssa_ir[i - 1].set_operand(0, current_operands[0]);
          ssa_ir[i] = IRInstruction<3>(
              Operator::NOOP, ValueType::UNKNOWN);
        }
      }
    }
  }
}
