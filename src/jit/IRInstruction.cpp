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
        : type_(type)
    {
      if (is_literal()) {
        throw BadOperandAccess(
            "Operand cannot be instantiated with specified type.");
      }
      std::memcpy(&storage_, &value, sizeof(std::size_t));
    }


    Operand::Operand(const Type type)
        : type_(type)
    {
    }


    Operand::Operand(const double value)
        : type_(Type::LITERAL_FLOAT)
    {
      std::memcpy(&storage_, &value, sizeof(double));
    }


    Operand::Operand(const bool value)
        : type_(Type::LITERAL_BOOLEAN)
    {
      std::memcpy(&storage_, &value, sizeof(bool));
    }


    Operand::Operand(const ObjectPtr value)
        : type_(Type::LITERAL_OBJECT)
    {
      std::memcpy(&storage_, &value, sizeof(ObjectPtr));
    }


    bool Operand::is_literal() const
    {
      return
          type_ == Type::LITERAL_BOOLEAN or
          type_ == Type::LITERAL_FLOAT or
          type_ == Type::LITERAL_OBJECT or
          type_ == Type::LITERAL_NIL;
    }


    template <>
    std::size_t unsafe_get<std::size_t>(const Operand& operand)
    {
      return *reinterpret_cast<const std::size_t*>(&operand.storage_);
    }


    template <>
    double unsafe_get<double>(const Operand& operand)
    {
      return *reinterpret_cast<const double*>(&operand.storage_);
    }


    template <>
    bool unsafe_get<bool>(const Operand& operand)
    {
      return *reinterpret_cast<const bool*>(&operand.storage_);
    }


    template <>
    ObjectPtr unsafe_get<ObjectPtr>(const Operand& operand)
    {
      return *reinterpret_cast<const ObjectPtr*>(&operand.storage_);
    }


    template <>
    std::size_t get<std::size_t>(const Operand& operand)
    {
      if (operand.is_literal()) {
        throw BadOperandAccess("Operand does not contain requested object.");
      }
      return *reinterpret_cast<const std::size_t*>(&operand.storage_);
    }


    template <>
    double get<double>(const Operand& operand)
    {
      if (operand.type_ != Operand::Type::LITERAL_FLOAT) {
        throw BadOperandAccess("Operand does not contain requested object.");
      }
      return *reinterpret_cast<const double*>(&operand.storage_);
    }


    template <>
    bool get<bool>(const Operand& operand)
    {
      if (operand.type_ != Operand::Type::LITERAL_BOOLEAN) {
        throw BadOperandAccess("Operand does not contain requested object.");
      }
      return *reinterpret_cast<const bool*>(&operand.storage_);
    }


    template <>
    ObjectPtr get<ObjectPtr>(const Operand& operand)
    {
      if (operand.type_ != Operand::Type::LITERAL_OBJECT) {
        throw BadOperandAccess("Operand does not contain requested object.");
      }
      return *reinterpret_cast<const ObjectPtr*>(&operand.storage_);
    }


    bool operator==(const Operand& first, const Operand& second)
    {
      if (first.type_ != second.type_) {
        return false;
      }
      return std::memcmp(&first.storage_, &second.storage_, 8);
    }
  }
}
