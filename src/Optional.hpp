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

#include <memory>
#include <type_traits>

#include "detail/common.hpp"


namespace loxx
{
  // A half-baked replacement for Boost optional
  template <typename T>
  class Optional
  {
  public:
    constexpr Optional();
    constexpr Optional(const Optional<T>& other);
    constexpr Optional(Optional<T>&& other) noexcept;

    ~Optional();

    template <typename U,
        typename = std::enable_if_t<
            not std::is_same<std::decay_t<U>, Optional<T>>::value>>
    constexpr Optional(U&& value) noexcept;

    template <typename U0, typename... Us>
    constexpr Optional(InPlace<U0>, Us&&... args);

    constexpr Optional<T>& operator=(const Optional<T>& other);
    constexpr Optional<T>& operator=(Optional<T>&& other) noexcept;

    template <typename U,
        typename = std::enable_if_t<
            not std::is_same<std::decay_t<U>, Optional<T>>::value>>
    constexpr Optional<T>& operator=(U&& value) noexcept;

    constexpr const T* operator->() const;
    constexpr T* operator->();

    constexpr const T& operator*() const { return value_; };
    constexpr const T& value() const;
    constexpr T& operator*() { return value_; }
    constexpr T& value();

    constexpr bool has_value() const { return has_value_; }
    constexpr explicit operator bool() const { return has_value_; }

    template <typename U>
    constexpr T value_or(U&& default_value) const;
    template <typename U>
    constexpr T value_or(U&& default_value);

    void reset();

  private:
    bool has_value_;

    union
    {
      T value_;
      struct {} no_value_;
    };
  };


  // Exception thrown when contents of a Optional instance can't be accessed
  class BadOptionalAccess : public std::exception
  {
  public:
    explicit BadOptionalAccess(const char* what) : what_(what) {}
    //explicit BadOptionalAccess(std::string what) : what_(std::move(what)) {}

    const char* what() const noexcept override { return what_; }

  private:
    const char* what_;
    //std::string what_;
  };


  template <typename T>
  constexpr Optional<T>::Optional()
      : has_value_(false), value_()
  {
  }


  template<typename T>
  constexpr Optional<T>::Optional(const Optional<T>& other)
      : has_value_(other.has_value_)
  {
    if (other.has_value_) {
      value_ = other.value_;
    }
    else {
      no_value_ = {};
    }
  }


  template<typename T>
  constexpr Optional<T>::Optional(Optional<T>&& other) noexcept
      : has_value_(other.has_value_)
  {
    if (has_value_) {
      value_ = other.value_;
    }

    other.has_value_ = false;
    other.no_value_ = {};
  }


  template<typename T>
  Optional<T>::~Optional()
  {
    reset();
  }


  template <typename T>
  template <typename U, typename>
  constexpr Optional<T>::Optional(U&& value) noexcept
      : has_value_(true), value_(value)
  {
  }


  template<typename T>
  template <typename U0, typename... Us>
  constexpr Optional<T>::Optional(InPlace<U0>, Us&&... args)
      : has_value_(true), value_(std::forward<Us>(args)...)
  {
  };


  template<typename T>
  constexpr Optional<T>& Optional<T>::operator=(const Optional<T>& other)
  {
    if (&other != this) {
      has_value_ = other.has_value_;
      if (other.has_value_) {
        value_ = other.value_;
      }
    }

    return *this;
  }


  template<typename T>
  constexpr Optional<T>& Optional<T>::operator=(Optional<T>&& other) noexcept
  {
    has_value_ = other.has_value_;

    if (has_value_) {
      value_ = other.value_;
    }

    other.has_value_ = false;
    other.no_value_ = {};
    return *this;
  }


  template<typename T>
  template<typename U, typename>
  constexpr Optional<T>& Optional<T>::operator=(U&& value) noexcept
  {
    has_value_ = true;
    value_ = T(std::forward<U>(value));
    return *this;
  }


  template<typename T>
  constexpr const T* Optional<T>::operator->() const
  {
    if (not has_value_) {
      throw BadOptionalAccess("Optional is not set.");
    }

    return &value_;
  }


  template<typename T>
  constexpr T* Optional<T>::operator->()
  {
    if (not has_value_) {
      throw BadOptionalAccess("Optional is not set.");
    }

    return &value_;
  }


  template<typename T>
  constexpr const T& Optional<T>::value() const
  {
    if (not has_value_) {
      throw BadOptionalAccess("Optional is not set.");
    }

    return value_;
  }


  template<typename T>
  constexpr T& Optional<T>::value()
  {
    if (not has_value_) {
      throw BadOptionalAccess("Optional is not set.");
    }

    return value_;
  }


  template<typename T>
  template<typename U>
  constexpr T Optional<T>::value_or(U&& default_value) const
  {
    if (has_value_) {
      return value();
    }
    return default_value;
  }


  template<typename T>
  template<typename U>
  constexpr T Optional<T>::value_or(U&& default_value)
  {
    if (has_value_) {
      return value();
    }
    return default_value;
  }


  template<typename T>
  void Optional<T>::reset()
  {
    if (has_value_) {
      (&value_)->~T();
    }
    has_value_ = false;
  }


  template <typename T>
  constexpr bool operator==(const Optional<T>& rhs,
                            const Optional<T>& lhs)
  {
    if (not (lhs.has_value() and rhs.has_value())) {
      return false;
    }

    return rhs.value() == lhs.value();
  }
}

#endif //LOXX_OPTIONAL_HPP
