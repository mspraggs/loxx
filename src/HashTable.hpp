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
    constexpr std::size_t default_size = 4;
    constexpr std::size_t growth_factor = 2;
    constexpr double load_factor = 0.75;
    constexpr std::size_t default_max_used_slots =
        static_cast<std::size_t>(default_size * load_factor);
  }


  template <typename Key, typename Value, typename Hash = std::hash<Key>,
            typename Compare = std::equal_to<Key>>
  class HashTable;


  template <typename Key, typename Value, typename Hash, typename Compare>
  class HashTableIterator
  {
    using Item = std::pair<Key, Value>;
    using Elem = Optional<Item>;
    using Iter = typename std::vector<Elem>::iterator;

  public:
    using difference_type   = std::ptrdiff_t;
    using value_type        = Item;
    using pointer           = Item*;
    using reference         = Item&;
    using iterator_category = std::forward_iterator_tag;

    HashTableIterator() = default;
    explicit HashTableIterator(Iter start, Iter last)
        : it_(start), finish_(last)
    {
      if (it_ != finish_ and not *it_) {
        ++(*this);
      }
    }

    auto operator++() -> HashTableIterator<Key, Value, Hash, Compare>&;
    auto operator++(int) -> HashTableIterator<Key, Value, Hash, Compare>;

    bool operator==(
        const HashTableIterator<Key, Value, Hash, Compare>& other) const;
    bool operator!=(
        const HashTableIterator<Key, Value, Hash, Compare>& other) const;

    auto operator*() -> reference;
    auto operator->() -> pointer;

  private:
    Item back_;
    Iter it_, finish_;
  };


  template <typename Key, typename Value, typename Hash, typename Compare>
  class HashTable
  {
    using Iter = HashTableIterator<Key, Value, Hash, Compare>;

  public:
    explicit HashTable()
        : num_used_slots_(0), max_used_slots_(detail::default_max_used_slots),
          mask_(detail::default_size - 1), data_(detail::default_size)
    {}

    Value& operator[](const Key& key);
    const Value& at(const Key& key) const;
    void erase(const Key& key);
    std::size_t count(const Key& key) const;
    bool has_item(const Key& key) const;

    std::size_t capacity() const { return data_.size(); }
    std::size_t size() const { return num_used_slots_; }

    auto begin() -> Iter;
    auto end() -> Iter;

  private:
    using Item = std::pair<Key, Value>;
    using Elem = Optional<Item>;

    void rehash();
    std::size_t find_pos(const Key& key, const std::size_t hash) const;
    std::size_t find_new_pos(const Key& key, const std::size_t hash) const;

    Hash hash_func_;
    Compare compare_;
    std::size_t num_used_slots_, max_used_slots_, mask_;
    std::vector<Elem> data_;
  };


  template <typename Key, typename Value, typename Hash, typename Compare>
  Value& HashTable<Key, Value, Hash, Compare>::operator[](const Key& key)
  {
    if (num_used_slots_ > max_used_slots_) {
      rehash();
    }

    const auto pos = find_new_pos(key, hash_func_(key));

    if (not data_[pos]) {
      data_[pos] = std::make_pair(key, Value());
      ++num_used_slots_;
    }

    return data_[pos]->second;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  const Value& HashTable<Key, Value, Hash, Compare>::at(const Key& key) const
  {
    const auto found_pos = find_pos(key, hash_func_(key));

    if (found_pos == data_.size()) {
      throw std::out_of_range("HashTable instance does not have supplied key!");
    }

    return data_[found_pos]->second;
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  void HashTable<Key, Value, Hash, Compare>::erase(const Key& key)
  {
    const auto hash = hash_func_(key);
    auto pos = find_pos(key, hash);

    if (pos == data_.size()) {
      return;
    }

    data_[pos].reset();
    --num_used_slots_;

    for (;;) {
      pos = (pos + 1) & mask_;

      if (not data_[pos]) {
        break;
      }

      const auto datum = std::move(data_[pos]);
      (*this)[datum->first] = datum->second;
    }
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  std::size_t HashTable<Key, Value, Hash, Compare>::count(const Key& key) const
  {
    return has_item(key) ? 1 : 0;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  bool HashTable<Key, Value, Hash, Compare>::has_item(const Key& key) const
  {
    const auto hash = hash_func_(key);
    const auto pos = find_pos(key, hash);
    return pos != data_.size();
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  void HashTable<Key, Value, Hash, Compare>::rehash()
  {
    const auto new_capacity = data_.size() * detail::growth_factor;
    mask_ = new_capacity - 1;
    max_used_slots_ *= detail::growth_factor;

    std::vector<Elem> old_data(new_capacity);
    std::swap(data_, old_data);

    for (auto& elem : old_data) {

      if (not elem) {
        continue;
      }

      const auto hash = hash_func_(elem->first);
      const auto new_pos = find_new_pos(elem->first, hash);
      data_[new_pos] = elem;
    }
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  std::size_t HashTable<Key, Value, Hash, Compare>::find_pos(
      const Key& key, const std::size_t hash) const
  {
    const auto init_pos = hash & mask_;

    auto pos = init_pos;
    do {
      if (data_[pos] and compare_(data_[pos]->first, key)) {
        return pos;
      }
      pos = (pos + 1) & mask_;
    } while (pos != init_pos);

    return data_.size();
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  std::size_t HashTable<Key, Value, Hash, Compare>::find_new_pos(
      const Key& key, const std::size_t hash) const
  {
    const auto init_pos = hash & mask_;

    auto pos = init_pos;
    do {
      if (not data_[pos] or compare_(data_[pos]->first, key)) {
        return pos;
      }
      pos = (pos + 1) & mask_;
    } while (pos != init_pos);

    return data_.size();
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTable<Key, Value, Hash, Compare>::begin() -> HashTable::Iter
  {
    return Iter(data_.begin(), data_.end());
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTable<Key, Value, Hash, Compare>::end() -> HashTable::Iter
  {
    return Iter(data_.end(), data_.end());
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTableIterator<Key, Value, Hash, Compare>::operator++()
      -> HashTableIterator<Key, Value, Hash, Compare>&
  {
    ++it_;
    while (it_ != finish_ and not *it_) {
      ++it_;
    }

    return *this;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTableIterator<Key, Value, Hash, Compare>::operator++(int)
      -> HashTableIterator<Key, Value, Hash, Compare>
  {
    const auto ret = *this;

    it_++;
    while (it_ != finish_ and not *it_) {
      it_++;
    }

    return ret;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  bool HashTableIterator<Key, Value, Hash, Compare>::operator==(
      const HashTableIterator<Key, Value, Hash, Compare>& other) const
  {
    return it_ == other.it_;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  bool HashTableIterator<Key, Value, Hash, Compare>::operator!=(
      const HashTableIterator<Key, Value, Hash, Compare>& other) const
  {
    return not (*this == other);
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTableIterator<Key, Value, Hash, Compare>::operator*() -> reference
  {
    if (it_ == finish_) {
      return back_;
    }

    return *(*it_);
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTableIterator<Key, Value, Hash, Compare>::operator->() -> pointer
  {
    if (it_ == finish_) {
      return &back_;
    }

    return &(*(*it_));
  }
}

#endif //LOXX_HASHTABLE_HPP
