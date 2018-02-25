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

#include <memory>
#include <type_traits>


namespace loxx
{
  // A half-baked replacement for the Boost variant
  template <typename... Ts>
  class Variant;

  // Helper functions
  namespace detail
  {
    template<typename T0>
    constexpr std::size_t type_index(std::size_t i = 0)
    {
      return i;
    }


    template<typename T0, typename T1, typename... Ts>
    constexpr std::size_t type_index(std::size_t i = 0)
    {
      return std::is_same<T0, T1>::value ? i : type_index<T0, Ts...>(i + 1);
    }


    constexpr std::size_t static_max()
    {
      return 0;
    }


    template<typename... Is>
    constexpr std::size_t static_max(std::size_t i0, Is... is)
    {
      return i0 > static_max(is...) ? i0 : static_max(is...);
    }


    template<std::size_t I, typename T0, typename... Ts>
    struct TypeIndexLookup : TypeIndexLookup<I - 1, Ts...>
    {
    };


    template<typename T0, typename... Ts>
    struct TypeIndexLookup<0, T0, Ts...>
    {
      using type = T0;
    };


    template<std::size_t I, typename... Ts>
    using LookupType = typename TypeIndexLookup<I, Ts...>::type;


    constexpr std::size_t index_lookup(std::size_t i,
                                       std::integer_sequence<bool>)
    {
      return i;
    }


    template <bool B0, bool... Bs>
    constexpr std::size_t index_lookup(std::size_t i,
                                       std::integer_sequence<bool, B0, Bs...>)
    {
      return B0 ? i : index_lookup(i + 1, std::integer_sequence<bool, Bs...>());
    }


    template <bool... Bs>
    constexpr std::size_t index_lookup(std::integer_sequence<bool, Bs...> seq)
    {
      return index_lookup(0, seq);
    }


    template <typename T0, typename... Ts>
    constexpr std::size_t convertible_type_index()
    {
      constexpr auto can_convert =
          std::integer_sequence<bool, std::is_convertible<T0, Ts>::value...>();
      return detail::index_lookup(can_convert);
    };


    template <typename T0, typename T1>
    bool validate_variants(const T0& first, const T1& second);


    template <typename T0, typename T1, typename... Ts>
    bool validate_variants(const T0& first, const T1& second,
                           const Ts&... subsequent);


    template <typename T0, typename... Ts>
    std::size_t common_index(const T0& first, const Ts&... subsequent);


    template <typename... Ts, typename... Us>
    auto variant_index_sequence(const Variant<Ts...>&, const Us&...)
        -> std::index_sequence_for<Ts...>
    {
      return std::index_sequence_for<Ts...>();
    };


    template <typename Ret, typename Fn, typename... Ts>
    Ret visit(Fn&&, const std::size_t, std::index_sequence<>,
              const Ts& ... operands);


    template <typename Ret, typename Fn,
              std::size_t I0, std::size_t... Is, typename... Ts>
    Ret visit(Fn&& func, const std::size_t shared_index,
              std::index_sequence<I0, Is...>, const Ts& ... operands);
  }


  template <typename T>
  struct InPlace
  {
    using type = T;
  };


  // Variant class template
  template <typename... Ts>
  class Variant
  {
  public:
    Variant();
    Variant(const Variant<Ts...>& other);
    Variant(Variant<Ts...>&& other) noexcept;

    template <typename T,
              typename = std::enable_if_t<
                  not std::is_same<std::decay_t<T>, Variant<Ts...>>::value>>
    Variant(T&& value) noexcept;

    template <typename T0, typename... Us>
    Variant(InPlace<T0>, Us&&... args);

    Variant<Ts...>& operator=(const Variant<Ts...>& other);
    Variant<Ts...>& operator=(Variant<Ts...>&& other) noexcept;

    template <typename T,
              typename = std::enable_if_t<
                  not std::is_same<std::decay_t<T>, Variant<Ts...>>::value>>
    Variant<Ts...>& operator=(T&& value) noexcept;

    std::size_t index() const { return type_index_; }

  private:
    template <std::size_t I, typename... Us>
    friend constexpr auto get(Variant<Us...>& variant)
        -> detail::LookupType<I, Us...>&;

    template <typename T, typename... Us>
    friend constexpr bool holds_alternative(Variant<Us...>& variant);

    std::size_t type_index_;
    std::aligned_storage_t<
        detail::static_max(sizeof(Ts)...),
        detail::static_max(alignof(Ts)...)> storage_;
  };


  // Exception thrown when contents of a Variant instance can't be accessed
  class BadVariantAccess : public std::exception
  {
  public:
    explicit BadVariantAccess(std::string what) : what_(std::move(what)) {}

    const char* what() const noexcept override { return what_.c_str(); }

  private:
    std::string what_;
  };


  template <typename... Ts>
  Variant<Ts...>::Variant()
      : type_index_(sizeof...(Ts))
  {
  }


  template<typename... Ts>
  Variant<Ts...>::Variant(const Variant<Ts...>& other)
      : type_index_(other.type_index_), storage_(other.storage_)
  {
  }


  template<typename... Ts>
  Variant<Ts...>::Variant(Variant<Ts...>&& other) noexcept
      : type_index_(other.type_index_), storage_(other.storage_)
  {
    other.type_index_ = sizeof...(Ts);
  }


  template <typename... Ts>
  template <typename T, typename>
  Variant<Ts...>::Variant(T&& value) noexcept
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
  Variant<Ts...>::Variant(InPlace<T0>, Us&&... args)
  {
    constexpr auto index = detail::type_index<T0, Ts...>();

    static_assert(index < sizeof...(Ts),
                  "Unable to construct variant using supplied type.");

    type_index_ = index;

    new (&storage_) T0(std::forward<Us>(args)...);
  };


  template<typename... Ts>
  Variant<Ts...>& Variant<Ts...>::operator=(const Variant<Ts...>& other)
  {
    if (&other != this) {
      type_index_ = other.type_index_;
      storage_ = other.storage_;
    }

    return *this;
  }


  template<typename... Ts>
  Variant<Ts...>& Variant<Ts...>::operator=(Variant<Ts...>&& other) noexcept
  {
    type_index_ = other.type_index_;
    storage_ = other.storage_;

    other.type_index_ = sizeof...(Ts);

    return *this;
  }


  template<typename... Ts>
  template<typename T, typename>
  Variant<Ts...>& Variant<Ts...>::operator=(T&& value) noexcept
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
  constexpr auto get(Variant<Ts...>& variant)
      -> detail::LookupType<I, Ts...>&
  {
    if (variant.type_index_ != I) {
      throw BadVariantAccess("Variant does not contain requested object.");
    }
    using T = detail::LookupType<I, Ts...>;
    return *reinterpret_cast<T*>(&variant.storage_);
  }


  template <std::size_t I, typename... Ts>
  constexpr auto get(const Variant<Ts...>& variant)
  {
    using T = detail::LookupType<I, Ts...>;
    return const_cast<const T&>(get<I>(const_cast<Variant<Ts...>&>(variant)));
  }


  // Functions to access data stored in Variant instances

  template <typename T0, typename... Ts>
  constexpr auto get(Variant<Ts...>& variant)
  {
    return get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <typename T0, typename... Ts>
  constexpr auto get(const Variant<Ts...>& variant)
  {
    return get<detail::type_index<T0, Ts...>()>(variant);
  }


  template <typename Fn, typename... Ts>
  auto visit(Fn&& func, const Ts&... variants)
  {
    using Ret = decltype(func(get<0>(variants)...));
    return detail::visit<Ret>(std::forward<Fn>(func),
                              detail::common_index(variants...),
                              detail::variant_index_sequence(variants...),
                              variants...);
  }


  template <typename T0, typename... Ts>
  constexpr bool holds_alternative(Variant<Ts...>& variant)
  {
    return variant.type_index_ == detail::type_index<T0, Ts...>();
  }


  template <typename T0, typename... Ts>
  constexpr bool holds_alternative(const Variant<Ts...>& variant)
  {
    return holds_alternative<T0>(const_cast<Variant<Ts...>&>(variant));;
  }


  template <typename... Ts>
  bool operator==(const Variant<Ts...>& rhs, const Variant<Ts...>& lhs)
  {
    if (lhs.index() != rhs.index()) {
      return false;
    }

    return visit(
        [] (auto elem1, auto elem2) { return elem1 == elem2; }, lhs, rhs);
  }


  template <typename T0, typename T1>
  bool detail::validate_variants(const T0& first, const T1& second)
  {
    return first.index() == second.index();
  }


  template <typename T0, typename T1, typename... Ts>
  bool detail::validate_variants(const T0& first, const T1& second,
                                 const Ts&... subsequent)
  {
    return first.index() == second.index() and
           detail::validate_variants(second, subsequent...);
  }


  template <typename T0, typename... Ts>
  std::size_t detail::common_index(const T0& first, const Ts&... subsequent)
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
}

#endif //LOXX_VARIANT_HPP
