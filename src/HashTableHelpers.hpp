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
 * Created by Matt Spraggs on 14/05/18.
 */

#ifndef LOXX_HASHTABLEHELPERS_HPP
#define LOXX_HASHTABLEHELPERS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>


namespace loxx
{
  namespace detail
  {
    enum class HashImpl { None, Pointer, Reference };

    template <typename T>
    class has_hash
    {
      using Yes = char;
      using No = Yes[2];

      template <typename C>
      static auto test_ref(const void*)
          -> decltype(std::size_t{std::declval<const C>().hash()}, Yes{});

      template <typename C>
      static No& test_ref(...);

      template <typename C>
      static auto test_pointer(const void*)
          -> decltype(std::size_t{std::declval<const C>()->hash()}, Yes{});

      template <typename C>
      static No& test_pointer(...);

      static constexpr bool from_reference =
          sizeof(test_ref<T>(nullptr)) == sizeof(Yes);
      static constexpr bool from_pointer =
          sizeof(test_pointer<T>(nullptr)) == sizeof(Yes);

    public:
      static constexpr HashImpl impl =
          from_pointer ?
          HashImpl::Pointer :
          (from_reference ? HashImpl::Reference : HashImpl::None);
    };

    template <typename Key, typename Hash>
    auto hash(Key&& key, Hash&&)
        -> std::enable_if_t<has_hash<Key>::impl == HashImpl::Reference,
                            std::size_t>
    {
      return key.hash();
    }

    template <typename Key, typename Hash>
    auto hash(Key&& key, Hash&&)
        -> std::enable_if_t<has_hash<Key>::impl == HashImpl::Pointer,
                            std::size_t>
    {
      return key->hash();
    }

    template <typename Key, typename Hash>
    auto hash(Key&& key, Hash&& hash_func)
        -> std::enable_if_t<has_hash<Key>::impl == HashImpl::None,
                            std::size_t>
    {
      return hash_func(std::forward<Key>(key));
    }


    template <typename Key>
    auto keys_equal(const Key& first, const Key& second)
        -> std::enable_if_t<not std::is_pointer<Key>::value, bool>
    {
      return first == second;
    }


    template <typename Key>
    auto keys_equal(const Key& first, const Key& second)
        -> std::enable_if_t<std::is_pointer<Key>::value, bool>
    {
      return *first == *second;
    }
  }



}

#endif //LOXX_HASHTABLEHELPERS_HPP
