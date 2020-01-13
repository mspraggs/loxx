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


    namespace detail
    {
      std::size_t combine_hashes(
          const std::size_t first, const std::size_t second);
    }


    Operand::Operand(const ValueType type)
        : type_(type), target_(reg_count_++)
    {
    }


    Operand::Operand(const Value& value)
        : type_(static_cast<ValueType>(value.index())), target_(&value)
    {
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


    std::size_t detail::combine_hashes(
        const std::size_t first, const std::size_t second)
    {
      return first + 0x9e3779b9 + (second << 6) + (second >> 2);
    }
  }
}
