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
 * Created by Matt Spraggs on 16/08/18.
 */

#ifndef LOXX_VARIANTIMPL_HPP
#define LOXX_VARIANTIMPL_HPP

#include <cstring>
#include <limits>
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


    template<bool B0, bool... Bs>
    constexpr std::size_t index_lookup(std::size_t i,
                                       std::integer_sequence<bool, B0, Bs...>)
    {
      return B0 ? i : index_lookup(i + 1, std::integer_sequence<bool, Bs...>());
    }


    template<bool... Bs>
    constexpr std::size_t index_lookup(std::integer_sequence<bool, Bs...> seq)
    {
      return index_lookup(0, seq);
    }


    template<typename T0, typename... Ts>
    constexpr std::size_t convertible_type_index()
    {
      constexpr auto can_convert =
          std::integer_sequence<bool, std::is_convertible<T0, Ts>::value...>();
      return detail::index_lookup(can_convert);
    };


    template<typename T0, typename T1>
    constexpr bool validate_variants(const T0& first, const T1& second);


    template<typename T0, typename T1, typename... Ts>
    constexpr bool validate_variants(const T0& first, const T1& second,
                                     const Ts& ... subsequent);


    template<typename T0, typename... Ts>
    constexpr std::size_t
    common_index(const T0& first, const Ts& ... subsequent);


    template<typename... Ts, typename... Us>
    auto variant_index_sequence(const Variant<Ts...>&, const Us& ...)
        -> std::index_sequence_for<Ts...>
    {
      return std::index_sequence_for<Ts...>();
    };


    template<typename Ret, typename Fn, typename... Ts>
    Ret visit(Fn&&, const std::size_t, std::index_sequence<>,
              const Ts& ... operands);


    template<typename Ret, typename Fn,
        std::size_t I0, std::size_t... Is, typename... Ts>
    Ret visit(Fn&& func, const std::size_t shared_index,
              std::index_sequence<I0, Is...>, const Ts& ... operands);


    template <typename T>
    constexpr bool all_types_trivial()
    {
      return std::is_trivial<T>::value;
    }


    template <typename T0, typename T1, typename... Ts>
    constexpr bool all_types_trivial()
    {
      return std::is_trivial<T0>::value and all_types_trivial<T1, Ts...>();
    }


    template<std::size_t N, typename... Ts>
    struct VariantHelperImpl;


    template<std::size_t N, typename T0, typename... Ts>
    struct VariantHelperImpl<N, T0, Ts...>
    {
      static void destroy(const std::size_t type_index, void* data);

      static void copy_construct(const std::size_t type_index,
                                 void* new_data, const void* old_data);

      static void move_construct(const std::size_t type_index,
                                 void* new_data, void* old_data);

      static void copy_assign(const std::size_t type_index,
                              void* new_data, const void* old_data);

      static void move_assign(const std::size_t type_index,
                              void* new_data, void* old_data);
    };


    template<std::size_t N>
    struct VariantHelperImpl<N>
    {
      static void destroy(const std::size_t, void*)
      {}

      static void copy_construct(const std::size_t, void*, const void*)
      {}

      static void move_construct(const std::size_t, void*, void*)
      {}

      static void copy_assign(const std::size_t, void*, const void*)
      {}

      static void move_assign(const std::size_t, void*, void*)
      {}
    };


    template<typename... Ts>
    using VariantHelper = VariantHelperImpl<sizeof...(Ts), Ts...>;


    template <bool Pod, std::size_t Size, typename... Ts>
    class VariantImpl
    {
    public:
      static void destroy(const std::size_t index, void* target);
      static void copy_construct(const std::size_t index,
                                 void* dest, const void* src);
      static void move_construct(const std::size_t index,
                                 void* dest, void* src);
      static void copy_assign(const std::size_t index,
                              void* dest, const void* src);
      static void move_assign(const std::size_t index,
                              void* dest, void* src);
    };


    template <std::size_t Size, typename... Ts>
    class VariantImpl<true, Size, Ts...>
    {
    public:
      static void destroy(const std::size_t index, void* target);
      static void copy_construct(const std::size_t index,
                                 void* dest, const void* src);
      static void move_construct(const std::size_t index,
                                 void* dest, void* src);
      static void copy_assign(const std::size_t index,
                              void* dest, const void* src);
      static void move_assign(const std::size_t index,
                              void* dest, void* src);
    };


    template<bool Pod, std::size_t Size, typename... Ts>
    void VariantImpl<Pod, Size, Ts...>::destroy(const std::size_t index,
                                                void* target)
    {
      VariantHelper<Ts...>::destroy(index, target);
    }


    template<bool Pod, std::size_t Size, typename... Ts>
    void VariantImpl<Pod, Size, Ts...>::copy_construct(
        const std::size_t index, void* dest, const void* src)
    {
      VariantHelper<Ts...>::copy_construct(index, dest, src);
    }


    template<bool Pod, std::size_t Size, typename... Ts>
    void VariantImpl<Pod, Size, Ts...>::move_construct(
        const std::size_t index, void* dest, void* src)
    {
      VariantHelper<Ts...>::move_construct(index, dest, src);
    }


    template<bool Pod, std::size_t Size, typename... Ts>
    void VariantImpl<Pod, Size, Ts...>::copy_assign(
        const std::size_t index, void* dest, const void* src)
    {
      VariantHelper<Ts...>::copy_assign(index, dest, src);
    }


    template<bool Pod, std::size_t Size, typename... Ts>
    void VariantImpl<Pod, Size, Ts...>::move_assign(
        const std::size_t index, void* dest, void* src)
    {
      VariantHelper<Ts...>::copy_assign(index, dest, src);
    }


    template<std::size_t Size, typename... Ts>
    void VariantImpl<true, Size, Ts...>::destroy(const std::size_t, void*)
    {
    }


    template<std::size_t Size, typename... Ts>
    void VariantImpl<true, Size, Ts...>::copy_construct(
        const std::size_t, void* dest, const void* src)
    {
      std::memcpy(dest, src, Size);
    }


    template<std::size_t Size, typename... Ts>
    void VariantImpl<true, Size, Ts...>::move_construct(
        const std::size_t, void* dest, void* src)
    {
      std::memcpy(dest, src, Size);
    }


    template<std::size_t Size, typename... Ts>
    void VariantImpl<true, Size, Ts...>::copy_assign(
        const std::size_t, void* dest, const void* src)
    {
      std::memcpy(dest, src, Size);
    }


    template<std::size_t Size, typename... Ts>
    void VariantImpl<true, Size, Ts...>::move_assign(
        const std::size_t, void* dest, void* src)
    {
      std::memcpy(dest, src, Size);
    }


    template<std::size_t N, typename T0, typename... Ts>
    void VariantHelperImpl<N, T0, Ts...>::destroy(
        const std::size_t type_index, void* data)
    {
      if (type_index == N - sizeof...(Ts) - 1) {
        reinterpret_cast<T0*>(data)->~T0();
        return;
      }
      detail::VariantHelperImpl<N, Ts...>::destroy(type_index, data);
    }


    template<std::size_t N, typename T0, typename... Ts>
    void VariantHelperImpl<N, T0, Ts...>::copy_construct(
        const std::size_t type_index, void* new_data, const void* old_data)
    {
      if (type_index == N - sizeof...(Ts) - 1) {
        new(new_data) T0(*reinterpret_cast<const T0*>(old_data));
        return;
      }
      VariantHelperImpl<N, Ts...>::copy_construct(type_index, new_data,
                                                  old_data);
    }


    template<std::size_t N, typename T0, typename... Ts>
    void VariantHelperImpl<N, T0, Ts...>::move_construct(
        const std::size_t type_index, void* new_data, void* old_data)
    {
      if (type_index == N - sizeof...(Ts) - 1) {
        new(new_data) T0(std::move(*reinterpret_cast<T0*>(old_data)));
        reinterpret_cast<T0*>(old_data)->~T0();
        return;
      }
      VariantHelperImpl<N, Ts...>::move_construct(type_index, new_data,
                                                  old_data);
    }


    template<std::size_t N, typename T0, typename... Ts>
    void VariantHelperImpl<N, T0, Ts...>::copy_assign(
        const std::size_t type_index, void* new_data, const void* old_data)
    {
      if (type_index == N - sizeof...(Ts) - 1) {
        *reinterpret_cast<T0*>(new_data) =
            *reinterpret_cast<const T0*>(old_data);
        return;
      }
      VariantHelperImpl<N, Ts...>::copy_assign(type_index, new_data, old_data);
    }


    template<std::size_t N, typename T0, typename... Ts>
    void VariantHelperImpl<N, T0, Ts...>::move_assign(
        const std::size_t type_index, void* new_data, void* old_data)
    {
      if (type_index == N - sizeof...(Ts) - 1) {
        *reinterpret_cast<T0*>(new_data) =
            std::move(*reinterpret_cast<const T0*>(old_data));
        reinterpret_cast<T0*>(old_data)->~T0();
        return;
      }
      VariantHelperImpl<N, Ts...>::move_assign(type_index, new_data, old_data);
    }
  }
}

#endif //LOXX_VARIANTIMPL_HPP
