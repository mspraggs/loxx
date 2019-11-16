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
 * Created by Matt Spraggs on 06/11/2019.
 */

#include <cassert>
#include <iomanip>
#include <iostream>

#include "SSAInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    std::size_t Operand::reg_count_ = 0;


    Operand::Operand(const ValueType type)
        : type_(type), target_(reg_count_++)
    {
    }


    Operand::Operand(const Value& value)
        : type_(static_cast<ValueType>(value.index())), target_(&value)
    {
    }


    SSAInstruction::SSAInstruction(
        const Operator op, const Operand operand0, const Operand operand1)
        : op_(op), operand0_(operand0), operand1_(operand1)
    {
    }
  }
}