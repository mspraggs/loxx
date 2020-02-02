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
 * Created by Matt Spraggs on 25/08/18.
 */

#ifndef LOXX_HASHIMPL_HPP
#define LOXX_HASHIMPL_HPP

#include <vector>

#include "../Optional.hpp"


namespace loxx
{
  namespace detail
  {
    constexpr std::size_t default_size = 4;
    constexpr std::size_t growth_factor = 2;
    constexpr double load_factor = 0.75;
    constexpr std::size_t default_max_used_slots =
        static_cast<std::size_t>(default_size * load_factor);


    template <typename Key, typename Elem, typename HashStruct>
    class HashImpl
    {
    protected:
      template <typename Fn>
      static void remove(HashStruct& obj, const Key& key, Fn add_func);

      static void rehash(HashStruct& obj);

      static std::size_t find_pos(
          const HashStruct& obj, const Key& key, const std::size_t hash);

      static std::size_t find_new_pos(
          const HashStruct& obj, const Key& key, const std::size_t hash);
    };


    template <typename Key, typename Elem, typename HashStruct>
    template <typename Fn>
    void HashImpl<Key, Elem, HashStruct>::remove(
        HashStruct& obj, const Key& key, Fn add_func)
    {
      const auto hash = obj.hash_func_(key);
      auto pos = find_pos(obj, key, hash);

      if (pos == obj.data_.size()) {
        return;
      }

      obj.data_[pos].reset();
      --obj.num_used_slots_;

      for (;;) {
        pos = (pos + 1) & obj.mask_;

        if (not obj.data_[pos]) {
          break;
        }

        const auto datum = std::move(obj.data_[pos]);
        add_func(obj, datum);
      }
    }


    template <typename Key, typename Elem, typename HashStruct>
    void HashImpl<Key, Elem, HashStruct>::rehash(HashStruct& obj)
    {
      const auto new_capacity = obj.data_.size() * growth_factor;
      obj.mask_ = new_capacity - 1;
      obj.max_used_slots_ *= detail::growth_factor;

      std::vector<Elem> old_data(new_capacity);
      std::swap(obj.data_, old_data);

      for (auto& elem : old_data) {

        if (not elem) {
          continue;
        }

        const auto hash = obj.hash_func_(obj.key_extractor_(elem));
        const auto new_pos = find_new_pos(obj, obj.key_extractor_(elem), hash);
        obj.data_[new_pos] = std::move(elem);
      }
    }


    template <typename Key, typename Elem, typename HashStruct>
    std::size_t HashImpl<Key, Elem, HashStruct>::find_pos(
        const HashStruct& obj, const Key& key, const std::size_t hash)
    {
      const auto init_pos = hash & obj.mask_;

      auto pos = init_pos;
      do {
        if (obj.data_[pos] and
            obj.compare_(obj.key_extractor_(obj.data_[pos]), key)) {
          return pos;
        }
        pos = (pos + 1) & obj.mask_;
      } while (pos != init_pos);

      return obj.data_.size();
    }


    template<typename Key, typename Elem, typename HashStruct>
    std::size_t HashImpl<Key, Elem, HashStruct>::find_new_pos(
        const HashStruct& obj, const Key& key, const std::size_t hash)
    {
      const auto init_pos = hash & obj.mask_;

      auto pos = init_pos;
      while (true) {
        if (not obj.data_[pos] or
            obj.compare_(obj.key_extractor_(obj.data_[pos]), key)) {
          return pos;
        }
        pos = (pos + 1) & obj.mask_;
      }
    }
  }
}

#endif //LOXX_HASHIMPL_HPP
