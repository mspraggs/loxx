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
 * Created by Matt Spraggs on 07/05/18.
 */

#ifndef LOXX_CODEOBJECT_HPP
#define LOXX_CODEOBJECT_HPP

#include <cstdint>
#include <tuple>
#include <vector>

#include "globals.hpp"
#include "StringHashTable.hpp"
#include "Value.hpp"


namespace loxx
{
  struct CodeObject
  {
    using InsPtr = std::vector<std::uint8_t>::const_iterator;

    std::vector<std::uint8_t> bytecode;
    ConstStringHashTable<InstrArgUByte> constant_map;
    std::vector<Value> constants;
    std::vector<std::tuple<std::int8_t, std::uint8_t>> line_num_table;
  };
}

#endif //LOXX_CODEOBJECT_HPP
