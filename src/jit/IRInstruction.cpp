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

#include "IRInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    Operand::Operand()
        : type_(Type::UNUSED)
    {
    }


    Operand::Operand(const Type type, const std::size_t value)
        : type_(type), index_(value)
    {
    }


    bool Operand::is_literal() const
    {
      return type_ == Type::LITERAL;
    }


    bool operator==(const Operand& first, const Operand& second)
    {
      if (first.type_ != second.type_) {
        return false;
      }
      return first.index_ == second.index_;
    }
  }
}
