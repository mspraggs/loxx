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

#include "../utils.hpp"

#include "SSAInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    std::size_t Operand::reg_count_ = 0;


    Operand::Operand(const ValueType value_type)
        : OperandBase(VirtualRegister{value_type, reg_count_++})
    {
    }


    ValueType Operand::value_type() const
    {
      switch (op_type()) {
      case Type::MEMORY:
        return static_cast<ValueType>(unsafe_get<const Value*>(*this)->index());
      case Type::REGISTER:
        return unsafe_get<VirtualRegister>(*this).type;
      case Type::IMMEDIATE:
        return static_cast<ValueType>(unsafe_get<Value>(*this).index());
      default:
        throw std::logic_error("invalid operand");
      }
    }


    std::size_t VirtualRegisterHasher::operator() (
        const VirtualRegister& value) const
    {
      return combine_hashes(static_cast<std::size_t>(value.type), value.index);
    }


    bool VirtualRegisterCompare::operator() (
        const VirtualRegister& first, const VirtualRegister& second) const
    {
      return first.type == second.type and first.index == second.index;
    }


    bool operator==(const VirtualRegister& first, const VirtualRegister& second)
    {
      return first.type == second.type and first.index == second.index;
    }
  }
}
