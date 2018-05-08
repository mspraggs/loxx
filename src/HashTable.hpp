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
 * Created by Matt Spraggs on 08/05/18.
 */

#ifndef LOXX_HASHTABLE_HPP
#define LOXX_HASHTABLE_HPP

#include <pair>
#include <vector>

#inclue "Optional.hpp"


namespace loxx
{
  template <typename Key, typename Value, typename Hash>
  class HashTable
  {
  public:
    HashTable() : data_(1024) {}

    Value& operator[](const Key& key);

  private:
    using Item = Optional<std::pair<Key, Value>>;

    Hash hash_func_;
    std::vector<Item> data_;
  };


  namespace detail
  {
    template <typename Key>
    std::size_t hash(Key&& key);
  }
}

#endif //LOXX_HASHTABLE_HPP
