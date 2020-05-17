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
 * Created by Matt Spraggs on 24/02/18.
 */

#ifndef LOXX_VARIANT_HPP
#define LOXX_VARIANT_HPP

#include <limits>
#include <memory>
#include <type_traits>

#include "detail/common.hpp"
#include "detail/VariantImpl.hpp"


namespace loxx
{
  // A half-baked replacement for the Boost variant

  // Variant class template
  template <typename... Ts>
  class Variant
      : private detail::VariantImpl<
          detail::all_types_trivial<Ts...>(),
          Variant<Ts...>, Ts...>
  {
    static_assert(sizeof...(Ts) < std::numeric_limits<std::uint8_t>::max(),
                  "Variant supports up to 255 types.");
  public:
    static constexpr std::uint8_t npos = sizeof...(Ts);

    constexpr Variant();

    constexpr Variant(const Variant<Ts...>& other);

    constexpr Variant(Variant<Ts...>&& other) noexcept;

    ~Variant();

    template <typename T,
        typename = std::enable_if_t<
            not std::is_same<std::decay_t<T>, Variant<Ts...>>::value>>
    constexpr Variant(T&& value) noexcept;

    template <typename T0, typename... Us>
    constexpr Variant(InPlace<T0>, Us&& ... args);

    constexpr Variant<Ts...>& operator=(const Variant<Ts...>& other);

    constexpr Variant<Ts...>& operator=(Variant<Ts...>&& other) noexcept;

    template <typename T,
        typename = std::enable_if_t<
            not std::is_same<std::decay_t<T>, Variant<Ts...>>::value>>
    constexpr Variant<Ts...>& operator=(T&& value) noexcept;

    constexpr std::size_t index() const { return type_index_; }

  private:
    template <std::size_t I, typename... Us>
    friend constexpr auto unsafe_get(Variant<Us...>& variant)
        -> detail::LookupType<I, Us...>&;

    template <std::size_t I, typename... Us>
    friend constexpr auto get(Variant<Us...>& variant)
        -> detail::LookupType<I, Us...>&;

    friend class detail::VariantImpl<
        detail::all_types_trivial<Ts...>(), Variant<Ts...>, Ts...>;

    friend struct VariantHasher;

    static constexpr std::size_t max_size = detail::static_max(sizeof(Ts)...);
    static constexpr std::size_t max_align = detail::static_max(alignof(Ts)...);

    std::size_t type_index_;
    std::aligned_storage_t<max_size, max_align> storage_;
  };


  struct VariantHasher
  {
    template <typename... Ts>
    std::size_t operator() (const Variant<Ts...>& value) const;
  };


  // Exception thrown when contents of a Variant instance can't be accessed
  class BadVariantAccess : public std::exception
  {
  public:
    explicit BadVariantAccess(std::string what)
        : std::exception(), what_(std::move(what))
    {}

    const char* what() const noexcept override { return what_.c_str(); }

  private:
    std::string what_;
  };


  template <typename... Ts>
  constexpr Variant<Ts...>::Variant()
      : type_index_(npos)
  {
  }


  template<typename... Ts>
  constexpr Variant<Ts...>::Variant(const Variant<Ts...>& other)
  {
    this->copy_construct(*this, other);
  }


  template<typename... Ts>
  constexpr Variant<Ts...>::Variant(Variant<Ts...>&& other) noexcept
  {
    if (index() != npos and index() != other.index()) {
      this->destroy(*this);
    }

    this->move_construct(*this, other);
  }


  template<typename... Ts>
  Variant<Ts...>::~Variant()
  {
    if (index() != npos) {
      this->destroy(*this);
    }
  }


  template <typename... Ts>
  template <typename T, typename>
  constexpr Variant<Ts...>::Variant(T&& value) noexcept
  {
    constexpr auto index = detail::convertible_type_index<T, Ts...>();

    static_assert(index < sizeof...(Ts),
                  "Unable to construct variant using supplied type.");

    type_index_ = index;
    using U = detail::LookupType<index, Ts...>;
    new (&storage_) U(std::forward<T>(value));
  }


  template<typename... Ts>
  template <typename T0, typename... Us>
  constexpr Variant<Ts...>::Variant(InPlace<T0>, Us&&... args)
  {
    constexpr auto index = detail::type_index<T0, Ts...>();

    static_assert(index < sizeof...(Ts),
                  "Unable to construct variant using supplied type.");

    type_index_ = index;
    new (&storage_) T0(std::forward<Us>(args)...);
  };


  template<typename... Ts>
  constexpr Variant<Ts...>& Variant<Ts...>::operator=(
      const Variant<Ts...>& other)
  {
    if (&other != this) {
      if (index() != npos and index() != other.index()) {
        this->destroy(*this);
      }

      this->copy_assign(*this, other);
    }

    return *this;
  }


  template<typename... Ts>
  constexpr Variant<Ts...>& Variant<Ts...>::operator=(
      Variant<Ts...>&& other) noexcept
  {
    if (index() != npos and index() != other.index()) {
      this->destroy(*this);
    }

    this->move_assign(*this, other);

    return *this;
  }


  template<typename... Ts>
  template<typename T, typename>
  constexpr Variant<Ts...>& Variant<Ts...>::operator=(T&& value) noexcept
  {
    constexpr auto index = detail::convertible_type_index<T, Ts...>();

    static_assert(index < sizeof...(Ts),
                  "Unable to construct variant using supplied type.");

    type_index_ = index;
    using U = detail::LookupType<index, Ts...>;
    new (&storage_) U(std::forward<T>(value));

    return *this;
  }


  template <std::size_t I, typename... Ts>
  constexpr auto get(Variant<Ts...>& variant) -> detail::LookupType<I, Ts...>&
  {
    if (variant.index() != I) {
      throw BadVariantAccess("Variant does not contain requested object.");
    }
    using T = detail::LookupType<I, Ts...>;
    return *reinterpret_cast<T*>(&variant.storage_);
  }


  template <std::size_t I, typename... Ts>
  constexpr decltype(auto) get(const Variant<Ts...>& variant)
  {
    using T = detail::LookupType<I, Ts...>;
    return const_cast<const T&>(get<I>(const_cast<Variant<Ts...>&>(variant)));
  }


  // Functions to access data stored in Variant instances

  template <typename T0, typename... Ts>
  constexpr decltype(auto) get(Variant<Ts...>& variant)
  {
    return get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <typename T0, typename... Ts>
  constexpr decltype(auto) get(const Variant<Ts...>& variant)
  {
    return get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <std::size_t I, typename... Ts>
  constexpr auto unsafe_get(Variant<Ts...>& variant)
      -> detail::LookupType<I, Ts...>&
  {
    static_assert(I < Variant<Ts...>::npos,
                  "Variant cannot contain requested type.");
    using T = detail::LookupType<I, Ts...>;
    return *reinterpret_cast<T*>(&variant.storage_);
  }


  template <std::size_t I, typename... Ts>
  constexpr decltype(auto) unsafe_get(const Variant<Ts...>& variant)
  {
    using T = detail::LookupType<I, Ts...>;
    return const_cast<const T&>(unsafe_get<I>(
        const_cast<Variant<Ts...>&>(variant)));
  }


  template <typename T0, typename... Ts>
  constexpr decltype(auto) unsafe_get(Variant<Ts...>& variant)
  {
    return unsafe_get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <typename T0, typename... Ts>
  constexpr decltype(auto) unsafe_get(const Variant<Ts...>& variant)
  {
    return unsafe_get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <typename Fn, typename... Ts>
  constexpr auto visit(Fn&& func, Ts&&... variants)
  {
    using Ret = decltype(func(get<0>(variants)...));
    return detail::visit<Ret>(std::forward<Fn>(func),
                              detail::common_index(variants...),
                              detail::variant_index_sequence(variants...),
                              std::forward<Ts>(variants)...);
  }


  template <typename T0, typename... Ts>
  constexpr bool holds_alternative(Variant<Ts...>& variant)
  {
    return variant.index() == detail::type_index<T0, Ts...>();
  }


  template <typename T0, typename... Ts>
  constexpr bool holds_alternative(const Variant<Ts...>& variant)
  {
    return variant.index() == detail::type_index<T0, Ts...>();
  }


  template <typename... Ts>
  constexpr bool operator==(const Variant<Ts...>& rhs,
                            const Variant<Ts...>& lhs)
  {
    if (lhs.index() != rhs.index()) {
      return false;
    }

    if (lhs.index() == Variant<Ts...>::npos) {
      return true;
    }

    return visit(
        [] (auto elem1, auto elem2) { return elem1 == elem2; }, lhs, rhs);
  }


  template <typename T>
  constexpr bool detail::validate_variants(const T&)
  {
    return true;
  }


  template <typename T0, typename T1>
  constexpr bool detail::validate_variants(const T0& first, const T1& second)
  {
    return first.index() == second.index();
  }


  template <typename T0, typename T1, typename... Ts>
  constexpr bool detail::validate_variants(const T0& first, const T1& second,
                                           const Ts&... subsequent)
  {
    return first.index() == second.index() and
           detail::validate_variants(second, subsequent...);
  }


  template <typename T0, typename... Ts>
  constexpr std::size_t detail::common_index(const T0& first,
                                             const Ts&... subsequent)
  {
    if (not validate_variants(first, subsequent...)) {
      throw BadVariantAccess("Unable to visit variants with mismatched types.");
    }
    return first.index();
  }


  template <typename Ret, typename Fn, typename... Ts>
  Ret detail::visit(Fn&&, const std::size_t, std::index_sequence<>,
                    const Ts&...)
  {
    throw BadVariantAccess("Unable to visit specified Variant instances.");
  }


  template <typename Ret, typename Fn,
      std::size_t I0, std::size_t... Is, typename... Ts>
  Ret detail::visit(Fn&& func, const std::size_t shared_index,
                    std::index_sequence<I0, Is...>, const Ts&... operands)
  {
    if (I0 == shared_index) {
      return func(get<I0>(operands)...);
    }
    return detail::visit<Ret>(std::forward<Fn>(func), shared_index,
                              std::index_sequence<Is...>(), operands...);
  }


  template <typename... Ts>
  std::size_t VariantHasher::operator() (const Variant<Ts...>& value) const
  {
    const auto type_hash = value.type_index_;
    const auto value_hash = visit(
        [] (auto elem) {
          return std::hash<decltype(elem)>()(elem);
        },
        value);
    return type_hash + 0x9e3779b9 + (value_hash << 6) + (value_hash >> 2);
  }
}

#endif //LOXX_VARIANT_HPP
