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
  class HashTable;


  template <typename Key, typename Value, typename Hash = std::hash<Key>>
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

    auto operator++() -> HashTableIterator<Key, Value, Hash>&;
    auto operator++(int) -> HashTableIterator<Key, Value, Hash>;

    bool operator==(const HashTableIterator<Key, Value, Hash>& other) const;
    bool operator!=(const HashTableIterator<Key, Value, Hash>& other) const;

    auto operator*() -> reference;
    auto operator->() -> pointer;

  private:
    Item back_;
    Iter it_, finish_;
  };


  template <typename Key, typename Value, typename Hash>
  class HashTable
  {
  public:
    HashTable()
        : num_free_slots_(detail::default_size),
          min_free_slots_(
              static_cast<std::size_t>(
                  (1 - detail::load_factor) * detail::default_size)),
          data_(detail::default_size)
    {}

    Value& operator[](const Key& key);
    void erase(const Key& key);

    std::size_t size() const { return data_.size() - num_free_slots_; }

    HashTableIterator<Key, Value, Hash> begin()
    { return HashTableIterator<Key, Value, Hash>(data_.begin(), data_.end()); };

    HashTableIterator<Key, Value, Hash> end()
    { return HashTableIterator<Key, Value, Hash>(data_.end(), data_.end()); };

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
    template <typename T>
    class has_hash
    {
      using Yes = char;
      using No = Yes[2];

      template <typename C>
      static auto test(const void*)
          -> decltype(std::size_t{std::declval<const C>().hash()}, Yes{});

      template <typename C>
      static No& test(...);

    public:
      static constexpr bool value = sizeof(test<T>(nullptr)) == sizeof(Yes);
    };

    template <typename Key, typename Hash>
    auto hash(Key&& key, Hash&& hash_func)
        -> std::enable_if_t<has_hash<Key>::value, std::size_t>
    {
      return key.hash();
    }

    template <typename Key, typename Hash>
    std::size_t hash(Key&& key, Hash&& hash_func)
    {
      return hash_func(std::forward<Key>(key));
    }
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
  void HashTable<Key, Value, Hash>::erase(const Key& key)
  {
    const auto hash = detail::hash(key, hash_func_);
    auto pos = find_pos(data_, key, hash);

    data_[pos] = {};

    for (;;) {
      pos = (pos + 1) % data_.size();

      if (not data_[pos]) {
        break;
      }

      const auto datum = std::move(data_[pos]);
      (*this)[datum->first] = datum->second;
    }
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
    min_free_slots_ =
        static_cast<std::size_t>(new_size * (1.0 - detail::load_factor));

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


  template <typename Key, typename Value, typename Hash>
  auto HashTableIterator<Key, Value, Hash>::operator++()
      -> HashTableIterator<Key, Value, Hash>&
  {
    ++it_;
    while (it_ != finish_ and not *it_) {
      ++it_;
    }

    return *this;
  }


  template <typename Key, typename Value, typename Hash>
  auto HashTableIterator<Key, Value, Hash>::operator++(int)
  -> HashTableIterator<Key, Value, Hash>
  {
    const auto ret = *this;

    it_++;
    while (it_ != finish_ and not *it_) {
      it_++;
    }

    return ret;
  }


  template <typename Key, typename Value, typename Hash>
  bool HashTableIterator<Key, Value, Hash>::operator==(
      const HashTableIterator<Key, Value, Hash>& other) const
  {
    return it_ == other.it_;
  }


  template <typename Key, typename Value, typename Hash>
  bool HashTableIterator<Key, Value, Hash>::operator!=(
      const HashTableIterator<Key, Value, Hash>& other) const
  {
    return not (*this == other);
  }


  template <typename Key, typename Value, typename Hash>
  auto HashTableIterator<Key, Value, Hash>::operator*() -> reference
  {
    if (it_ == finish_) {
      return back_;
    }

    return *(*it_);
  }


  template <typename Key, typename Value, typename Hash>
  auto HashTableIterator<Key, Value, Hash>::operator->() -> pointer
  {
    if (it_ == finish_) {
      return &back_;
    }

    return &(*(*it_));
  }
}

#endif //LOXX_HASHTABLE_HPP
