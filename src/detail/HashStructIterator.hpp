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
 * Created by Matt Spraggs on 26/08/18.
 */

#ifndef LOXX_HASHSTRUCTITERATOR_HPP
#define LOXX_HASHSTRUCTITERATOR_HPP

#include <type_traits>
#include <vector>

#include "../Optional.hpp"


namespace loxx
{
  namespace detail
  {
    template<typename Item, typename Hash, typename Compare>
    class HashStructIterator
    {
      constexpr static bool is_const = std::is_const<Item>::value;
      using BareItem = typename std::remove_const<Item>::type;
      using BareElem = Optional<BareItem>;
      using Elem =
          typename std::conditional<
            is_const,
            const BareElem,
            BareElem
          >::type;
      using Iter =
          typename std::conditional<
            is_const,
            typename std::vector<BareElem>::const_iterator,
            typename std::vector<BareElem>::iterator
          >::type;

    public:
      using difference_type   = std::ptrdiff_t;
      using value_type        = Item;
      using pointer           = Item*;
      using reference         = Item&;
      using iterator_category = std::forward_iterator_tag;

      HashStructIterator() = default;

      explicit HashStructIterator(Iter start, Iter last)
          : it_(start), finish_(last)
      {
        if (it_ != finish_ and not*it_) {
          ++(*this);
        }
      }

      auto operator++() -> HashStructIterator<Item, Hash, Compare>&;

      auto operator++(int) -> HashStructIterator<Item, Hash, Compare>;

      bool operator==(
          const HashStructIterator<Item, Hash, Compare>& other) const;

      bool operator!=(
          const HashStructIterator<Item, Hash, Compare>& other) const;

      auto operator*() -> reference;
      auto operator*() const -> const reference;

      auto operator->() -> pointer;
      auto operator->() const -> const pointer;

    private:
      Item back_;
      Iter it_, finish_;
    };

    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator++()
        -> HashStructIterator<Item, Hash, Compare>&
    {
      ++it_;
      while (it_ != finish_ and not *it_) {
        ++it_;
      }

      return *this;
    }


    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator++(int)
        -> HashStructIterator<Item, Hash, Compare>
    {
      const auto ret = *this;

      it_++;
      while (it_ != finish_ and not *it_) {
        it_++;
      }

      return ret;
    }


    template <typename Item, typename Hash, typename Compare>
    bool HashStructIterator<Item, Hash, Compare>::operator==(
        const HashStructIterator<Item, Hash, Compare>& other) const
    {
      return it_ == other.it_;
    }


    template <typename Item, typename Hash, typename Compare>
    bool HashStructIterator<Item, Hash, Compare>::operator!=(
        const HashStructIterator<Item, Hash, Compare>& other) const
    {
      return not (*this == other);
    }


    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator*() -> reference
    {
      if (it_ == finish_) {
        return back_;
      }

      return *(*it_);
    }


    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator*() const
        -> const reference
    {
      if (it_ == finish_) {
        return back_;
      }

      return *(*it_);
    }


    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator->() -> pointer
    {
      if (it_ == finish_) {
        return &back_;
      }

      return &(*(*it_));
    }


    template <typename Item, typename Hash, typename Compare>
    auto HashStructIterator<Item, Hash, Compare>::operator->() const
        -> const pointer
    {
      if (it_ == finish_) {
        return &back_;
      }

      return &(*(*it_));
    }
  }
}

#endif //LOXX_HASHSTRUCTITERATOR_HPP
