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
 * Created by Matt Spraggs on 28/03/2018.
 */

#ifndef LOXX_VALUE_HPP
#define LOXX_VALUE_HPP

#include <ios>
#include <string>
#include <unordered_map>
#include <vector>

#include "globals.hpp"
#include "Variant.hpp"

namespace loxx
{
  enum class ValueType {
    FLOAT = 0,
    BOOLEAN = 1,
    OBJECT = 2,
    UNKNOWN = 3,
  };

  class Object;

  using Value = Variant<double, bool, Object*>;


  template <typename Os>
  Os& operator<<(Os& os, const ValueType value_type)
  {
    switch (value_type) {

    case ValueType::FLOAT:
      os << "FLOAT";
      break;

    case ValueType::BOOLEAN:
      os << "BOOL";
      break;

    case ValueType::OBJECT:
      os << "OBJ";
      break;

    case ValueType::UNKNOWN:
      os << "---";
      break;
    }

    return os;
  }


  template <typename OStream, typename T>
  auto operator<<(OStream& os, const T& value)
      -> std::enable_if_t<std::is_same<T, Value>::value, OStream&>;
}

#endif // LOXX_VALUE_HPP
