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
 * Created by Matt Spraggs on 27/08/18.
 */

#ifndef LOXX_UTILS_HPP
#define LOXX_UTILS_HPP

#include <cstdint>
#include <vector>

#include "CodeObject.hpp"

namespace loxx
{
  template <typename T>
  T read_integer_at_pos(const CodeObject::InsPtr pos)
  {
    T integer;
    std::copy(pos, pos + sizeof(T), reinterpret_cast<std::uint8_t*>(&integer));
    return integer;
  }
}

#endif //LOXX_UTILS_HPP
