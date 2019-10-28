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

#ifndef LOXX_STRINGHASHTABLE_HPP
#define LOXX_STRINGHASHTABLE_HPP

#include "HashSet.hpp"
#include "HashTable.hpp"


namespace loxx
{
  class StringObject;

  struct HashStringObject
  {
    std::size_t operator()(const StringObject* obj) const;
  };


  struct CompareStringObject
  {
    bool operator() (
        const StringObject* first, const StringObject* second) const;
  };

  template <typename T>
  using ConstStringHashTable =
      HashTable<const StringObject*, T, HashStringObject, CompareStringObject>;


  template <typename T>
  using StringHashTable =
      HashTable<StringObject*, T, HashStringObject, CompareStringObject>;


  using StringHashSet =
      HashSet<StringObject*, HashStringObject, CompareStringObject>;
}

#endif //LOXX_STRINGHASHTABLE_HPP
