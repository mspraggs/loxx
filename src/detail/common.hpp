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

#ifndef LOXX_COMMON_HPP
#define LOXX_COMMON_HPP

#include <cstdint>
#include <vector>

namespace loxx
{
  template <typename T>
  struct InPlace
  {
    using type = T;
  };


  template <typename T>
  T read_integer_at_pos(const std::vector<std::uint8_t>& bytecode,
                        const std::size_t pos)
  {
    const T integer = *reinterpret_cast<const T*>(&bytecode[pos]);
    return integer;
  }
}

#endif //LOXX_COMMON_HPP
