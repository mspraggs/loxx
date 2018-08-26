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

#ifndef LOXX_HASHSET_HPP
#define LOXX_HASHSET_HPP

#include <functional>
#include <utility>
#include <vector>

#include "detail/HashStructIterator.hpp"
#include "detail/HashImpl.hpp"
#include "Optional.hpp"


namespace loxx
{
  template <typename Key, typename Hash = std::hash<Key>,
            typename Compare = std::equal_to<Key>>
  class HashSet
      : private detail::HashImpl<
          Key,
          Optional<Key>,
          HashSet<Key, Hash, Compare>>
  {
  friend class detail::HashImpl<
      Key,
      Optional<Key>,
      HashSet<Key, Hash, Compare>>;

  public:
    using Iter = detail::HashStructIterator<Key, Hash, Compare>;
    using Elem = Optional<Key>;

    explicit HashSet()
        : num_used_slots_(0), max_used_slots_(detail::default_max_used_slots),
          mask_(detail::default_size - 1), data_(detail::default_size)
    {}
    void insert(const Key& key);
    const Elem& get(const Key& key) const;
    template <typename Fn>
    const Elem& find(const Key& key, Fn evaluate) const;
    void erase(const Key& key);
    std::size_t count(const Key& key) const;
    bool has_item(const Key& key) const;

    std::size_t capacity() const { return data_.size(); }
    std::size_t size() const { return num_used_slots_; }

    auto begin() -> Iter;
    auto end() -> Iter;

  private:
    struct KeyExtractor
    {
      const Key& operator()(const Elem& elem) const { return *elem; }
    };

    Hash hash_func_;
    Compare compare_;
    KeyExtractor key_extractor_;
    std::size_t num_used_slots_, max_used_slots_, mask_;
    std::vector<Elem> data_;
  };


  template <typename Key, typename Hash, typename Compare>
  void HashSet<Key, Hash, Compare>::insert(const Key& key)
  {
    if (num_used_slots_ >= max_used_slots_) {
      this->rehash(*this);
    }

    const auto pos = this->find_new_pos(*this, key, hash_func_(key));

    if (not data_[pos]) {
      data_[pos] = key;
      ++num_used_slots_;
    }
  }


  template <typename Key, typename Hash, typename Compare>
  auto HashSet<Key, Hash, Compare>::get(const Key& key) const -> const Elem&
  {
    const auto found_pos = this->find_new_pos(*this, key, hash_func_(key));
    return data_[found_pos];
  }


  template<typename Key, typename Hash, typename Compare>
  template<typename Fn>
  auto HashSet<Key, Hash, Compare>::find(
      const Key& key, Fn evaluate) const -> const Elem&
  {
    auto pos = hash_func_(key) & mask_;

    while (data_[pos]) {
      if (evaluate(*data_[pos])) {
        return data_[pos];
      }
      pos = (pos + 1) & mask_;
    }

    return data_[pos];
  }


  template<typename Key, typename Hash, typename Compare>
  void HashSet<Key, Hash, Compare>::erase(const Key& key)
  {
    using HashStruct = HashSet<Key, Hash, Compare>;
    const auto add_func =
        [] (HashStruct& obj, const Elem& new_elem) {
          obj.insert(*new_elem);
        };
    this->remove(*this, key, add_func);
  }


  template <typename Key, typename Hash, typename Compare>
  std::size_t HashSet<Key, Hash, Compare>::count(const Key& key) const
  {
    return has_item(key) ? 1 : 0;
  }


  template <typename Key, typename Hash, typename Compare>
  bool HashSet<Key, Hash, Compare>::has_item(const Key& key) const
  {
    const auto hash = hash_func_(key);
    const auto pos = this->find_pos(*this, key, hash);
    return pos != data_.size();
  }


  template <typename Key, typename Hash, typename Compare>
  auto HashSet<Key, Hash, Compare>::begin() -> HashSet::Iter
  {
    return Iter(data_.begin(), data_.end());
  }


  template <typename Key, typename Hash, typename Compare>
  auto HashSet<Key, Hash, Compare>::end() -> HashSet::Iter
  {
    return Iter(data_.end(), data_.end());
  }
}

#endif //LOXX_HASHSET_HPP
