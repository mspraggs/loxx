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

    inline std::size_t base_2_mod(const std::size_t value, const size_t div)
    {
      return value & (div - 1);
    }
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
    explicit HashTable(Hash hash_func = Hash(), Compare compare = Compare())
        : hash_func_(hash_func), compare_(compare),
          num_free_slots_(detail::default_size),
          min_free_slots_(
              static_cast<std::size_t>(
                  (1 - detail::load_factor) * detail::default_size)),
          data_(detail::default_size)
    {}

    Value& operator[](const Key& key);
    const Value& at(const Key& key) const;
    void erase(const Key& key);
    std::size_t count(const Key& key) const;
    bool has_item(const Key& key) const;

    std::size_t capacity() const { return data_.size(); }
    std::size_t size() const { return data_.size() - num_free_slots_; }

    auto begin() -> Iter;
    auto end() -> Iter;

  private:
    using Item = std::pair<Key, Value>;
    using Elem = Optional<Item>;

    void rehash();
    Optional<std::size_t> find_pos(
        const std::vector<Elem>& data, const Key& key,
        const std::size_t hash) const;
    std::size_t find_new_pos(
        const std::vector<Elem>& data, const Key& key,
        const std::size_t hash) const;

    Hash hash_func_;
    Compare compare_;
    std::size_t num_free_slots_, min_free_slots_;
    std::vector<Elem> data_;
  };


  template <typename Key, typename Value, typename Hash, typename Compare>
  Value& HashTable<Key, Value, Hash, Compare>::operator[](const Key& key)
  {
    if (num_free_slots_ < min_free_slots_) {
      rehash();
    }

    const auto pos = find_new_pos(data_, key, hash_func_(key));

    if (not data_[pos]) {
      data_[pos] = std::make_pair(key, Value());
      --num_free_slots_;
    }

    return data_[pos]->second;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  const Value& HashTable<Key, Value, Hash, Compare>::at(const Key& key) const
  {
    const auto found_pos = find_pos(data_, key, hash_func_(key));

    if (not found_pos) {
      throw std::out_of_range("HashTable instance does not have supplied key!");
    }

    return data_[*found_pos]->second;
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  void HashTable<Key, Value, Hash, Compare>::erase(const Key& key)
  {
    const auto hash = hash_func_(key);
    const auto found_pos = find_pos(data_, key, hash);

    if (not found_pos) {
      return;
    }

    const auto size = data_.size();
    auto pos = *found_pos;
    data_[pos] = {};
    --num_free_slots_;

    for (;;) {
      pos = detail::base_2_mod(pos + 1, size);

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
    const auto pos = find_pos(data_, key, hash);
    return pos.has_value();
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  void HashTable<Key, Value, Hash, Compare>::rehash()
  {
    const auto new_size =
        static_cast<std::size_t>(data_.size() * detail::growth_factor);

    std::vector<Elem> new_data(new_size);

    for (auto& elem : data_) {

      if (not elem) {
        continue;
      }

      const auto hash = hash_func_(elem->first);
      const auto new_pos = find_new_pos(new_data, elem->first, hash);
      new_data[new_pos] = elem;
    }

    num_free_slots_ += new_size - data_.size();
    min_free_slots_ =
        static_cast<std::size_t>(new_size * (1.0 - detail::load_factor));

    std::swap(data_, new_data);
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  Optional<std::size_t> HashTable<Key, Value, Hash, Compare>::find_pos(
      const std::vector<Elem>& data, const Key& key,
      const std::size_t hash) const
  {
    const auto size = data.size();
    const auto init_pos = detail::base_2_mod(hash, size);

    auto pos = init_pos;
    do {
      if (data[pos] and compare_(data[pos]->first, key)) {
        return pos;
      }
      pos = detail::base_2_mod(pos + 1, size);
    } while (pos != init_pos);

    return {};
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  std::size_t HashTable<Key, Value, Hash, Compare>::find_new_pos(
      const std::vector<Elem>& data, const Key& key,
      const std::size_t hash) const
  {
    const auto size = data.size();
    const auto init_pos = detail::base_2_mod(hash, size);

    auto pos = init_pos;
    do {
      if (not data[pos] or compare_(data[pos]->first, key)) {
        return pos;
      }
      pos = detail::base_2_mod(pos + 1, size);
    } while (pos != init_pos);

    throw std::out_of_range("HashTable instance does not have supplied key!");
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
