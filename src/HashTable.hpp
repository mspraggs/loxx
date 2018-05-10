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
  template <typename Key, typename Value, typename Hash = std::hash<Key>>
  class HashTable
  {
    constexpr std::size_t default_size_ = 1024;
  public:
    HashTable()
        : num_free_slots_(default_size), data_set_(default_size_),
          data_(default_size_)
    {}

    Value& operator[](const Key& key);

  private:
    using Item = std::pair<Key, Value>;

    void rehash();

    Hash hash_func_;
    std::size_t num_free_slots_;
    std::vector<bool> data_set_;
    std::vector<Item> data_;
  };


  namespace detail
  {
    template <typename Key, typename Hash>
    std::size_t hash(Key&& key, Hash&& hash_func);
  }


  template <typename Key, typename Value, typename Hash>
  Value& HashTable::operator[](const Key& key)
  {
    if (num_free_slots_ == 0) {
      rehash();
    }

    auto pos = detail::hash(key, hash_func) % data_.size();

    while (not data_set_[pos]) {
      pos = (pos + 1) % data.size();
    }
    data_set_[pos] = true;
    return data[pos]
  }


  namespace detail
  {
    template <typename Key, typename Hash>
    std::size_t hash(Key&& key, Hash&& hash_func)
    {
      return hash_func(key);
    }
  }  
}

#endif //LOXX_HASHTABLE_HPP
