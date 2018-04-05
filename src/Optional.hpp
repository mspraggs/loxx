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
 * Created by Matt Spraggs on 05/04/18.
 */

#ifndef LOXX_OPTIONAL_HPP
#define LOXX_OPTIONAL_HPP

#include "Variant.hpp"


namespace loxx
{
  template <typename T>
  class Optional : private Variant<T>
  {
  public:
    using Variant<T>::Variant;
    using Variant<T>::operator=;

    constexpr explicit operator bool() const noexcept
    { return this->index() != Variant<T>::npos; }

    constexpr decltype(auto) operator->() const { return &get<T>(*this); }
    constexpr decltype(auto) operator->() { return &get<T>(*this); }

    constexpr decltype(auto) operator*() const { return get<T>(*this); }
    constexpr decltype(auto) operator*() { return get<T>(*this); }

    constexpr bool has_value() const noexcept
    { return static_cast<bool>(*this); }

    constexpr const T& value() const { return this->operator*(); }
    constexpr T& value() { return this->operator*(); }

    template <typename U>
    constexpr T value_or(U&& default_value);
    template <typename U>
    constexpr T value_or(U&& default_value) const;

    void reset();
  };


  template<typename T>
  template<typename U>
  constexpr T Optional<T>::value_or(U&& default_value)
  {
    if (has_value()) {
      return value();
    }
    return default_value;
  }


  template<typename T>
  template<typename U>
  constexpr T Optional<T>::value_or(U&& default_value) const
  {
    if (has_value()) {
      return value();
    }
    return default_value;
  }


  template<typename T>
  void Optional<T>::reset()
  {
    *this = Variant<T>();
  }
}

#endif //LOXX_OPTIONAL_HPP
