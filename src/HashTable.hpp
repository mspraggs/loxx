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

#include <functional>
#include <utility>
#include <vector>

#include "detail/HashImpl.hpp"
#include "detail/HashStructIterator.hpp"
#include "Optional.hpp"


namespace loxx
{
  template <typename Key, typename Value, typename Hash = std::hash<Key>,
            typename Compare = std::equal_to<Key>>
  class HashTable
    : private detail::HashImpl<
        Key,
        Optional<std::pair<Key, Value>>,
        HashTable<Key, Value, Hash, Compare>>
  {
    friend class detail::HashImpl<
        Key,
        Optional<std::pair<Key, Value>>,
        HashTable<Key, Value, Hash, Compare>>;

  public:
    using Item = std::pair<Key, Value>;
    using Elem = Optional<Item>;
    using Iter = detail::HashStructIterator<Item, Hash, Compare>;
    using ConstIter = detail::HashStructIterator<const Item, Hash, Compare>;

    explicit HashTable()
        : num_used_slots_(0), max_used_slots_(detail::default_max_used_slots),
          mask_(detail::default_size - 1), data_(detail::default_size)
    {}

    Value& operator[](const Key& key);
    const Value& at(const Key& key) const;
    const Elem& get(const Key& key) const;
    Item& insert(const Key& key, const Value& value = Value());
    void erase(const Key& key);
    std::size_t count(const Key& key) const;
    bool has_item(const Key& key) const;

    std::size_t capacity() const { return data_.size(); }
    std::size_t size() const { return num_used_slots_; }

    auto begin() -> Iter;
    auto end() -> Iter;
    auto begin() const -> ConstIter;
    auto end() const -> ConstIter;

  private:
    struct KeyExtractor
    {
      const Key& operator()(const Elem& elem) const { return elem->first; }
    };

    Hash hash_func_;
    Compare compare_;
    KeyExtractor key_extractor_;
    std::size_t num_used_slots_, max_used_slots_, mask_;
    std::vector<Elem> data_;
  };


  template <typename Key, typename Value, typename Hash, typename Compare>
  Value& HashTable<Key, Value, Hash, Compare>::operator[](const Key& key)
  {
    if (num_used_slots_ >= max_used_slots_) {
      this->rehash(*this);
    }

    const auto pos = this->find_new_pos(*this, key, hash_func_(key));

    if (not data_[pos]) {
      data_[pos] = std::make_pair(key, Value());
      ++num_used_slots_;
    }

    return data_[pos]->second;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  const Value& HashTable<Key, Value, Hash, Compare>::at(const Key& key) const
  {
    const auto found_pos = this->find_pos(*this, key, hash_func_(key));

    if (found_pos == data_.size()) {
      throw std::out_of_range("HashTable instance does not have supplied key!");
    }

    return data_[found_pos]->second;
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTable<Key, Value, Hash, Compare>::get(const Key& key) const
      -> const Elem&
  {
    const auto found_pos = this->find_new_pos(*this, key, hash_func_(key));
    return data_[found_pos];
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTable<Key, Value, Hash, Compare>::insert(
      const Key& key, const Value& value) -> Item&
  {
    if (num_used_slots_ >= max_used_slots_) {
      this->rehash(*this);
    }

    const auto pos = this->find_new_pos(*this, key, hash_func_(key));

    if (not data_[pos]) {
      data_[pos] = std::make_pair(key, value);
      ++num_used_slots_;
    }

    return *data_[pos];
  }


  template<typename Key, typename Value, typename Hash, typename Compare>
  void HashTable<Key, Value, Hash, Compare>::erase(const Key& key)
  {
    using HashStruct = HashTable<Key, Value, Hash, Compare>;
    const auto add_func =
        [] (HashStruct& obj, const Elem& new_elem) {
          obj[new_elem->first] = new_elem->second;
        };
    this->remove(*this, key, add_func);
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
    const auto pos = this->find_pos(*this, key, hash);
    return pos != data_.size();
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
  auto HashTable<Key, Value, Hash, Compare>::begin() const
      -> HashTable::ConstIter
  {
    return ConstIter(data_.cbegin(), data_.cend());
  }


  template <typename Key, typename Value, typename Hash, typename Compare>
  auto HashTable<Key, Value, Hash, Compare>::end() const
      -> HashTable::ConstIter
  {
    return ConstIter(data_.cend(), data_.cend());
  }
}

#endif //LOXX_HASHTABLE_HPP
