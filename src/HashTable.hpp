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

#include <utility>
#include <vector>

#include "Optional.hpp"


namespace loxx
{
  namespace detail
  {
    constexpr std::size_t default_size = 1024;
    constexpr double load_factor = 0.75;
    constexpr double growth_factor = 2.0;
  }

  template <typename Key, typename Value, typename Hash = std::hash<Key>>
  class HashTable
  {
  public:
    HashTable()
        : num_free_slots_(detail::default_size),
          min_free_slots_(static_cast<std::size_t>(
                              detail::load_factor * detail::default_size)),
          data_(detail::default_size)
    {}

    Value& operator[](const Key& key);

  private:
    using Item = std::pair<Key, Value>;
    using Elem = Optional<Item>;

    void rehash();
    static std::size_t find_pos(
        const std::vector<Elem>& data, const Key& key, const std::size_t hash);

    Hash hash_func_;
    std::size_t num_free_slots_, min_free_slots_;
    std::vector<Elem> data_;
  };


  namespace detail
  {
    template <typename Key, typename Hash>
    std::size_t hash(Key&& key, Hash&& hash_func);
  }


  template <typename Key, typename Value, typename Hash>
  Value& HashTable<Key, Value, Hash>::operator[](const Key& key)
  {
    if (num_free_slots_ < min_free_slots_) {
      rehash();
    }

    const auto pos = find_pos(data_, key, detail::hash(key, hash_func_));

    if (not data_[pos]) {
      data_[pos] = std::make_pair(key, Value());
      --num_free_slots_;
    }

    return data_[pos]->second;
  }


  template<typename Key, typename Value, typename Hash>
  void HashTable<Key, Value, Hash>::rehash()
  {
    const auto new_size =
        static_cast<std::size_t>(data_.size() * detail::growth_factor);

    std::vector<Elem> new_data(new_size);

    for (auto& elem : data_) {

      if (not elem) {
        continue;
      }

      const auto hash = detail::hash(elem->first, hash_func_);
      const auto new_pos = find_pos(new_data, elem->first, hash);
      new_data[new_pos] = elem;
    }

    num_free_slots_ += new_size - data_.size();
    min_free_slots_ = static_cast<std::size_t>(new_size * detail::load_factor);

    std::swap(data_, new_data);
  }


  template<typename Key, typename Value, typename Hash>
  std::size_t HashTable<Key, Value, Hash>::find_pos(
      const std::vector<Elem>& data, const Key& key, const std::size_t hash)
  {
    auto pos = hash % data.size();

    while (data[pos] and data[pos]->first != key) {
      pos = (pos + 1) % data.size();
    }

    return pos;
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
