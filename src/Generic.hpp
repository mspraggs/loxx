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
 * Created by Matt Spraggs on 01/11/17.
 */

#ifndef LOXX_GENERIC_HPP
#define LOXX_GENERIC_HPP

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>


namespace loxx
{
  namespace detail
  {
    template <typename T>
    class Comparable
    {
      template <typename C>
      static char test(decltype(&C::operator==));
      template <typename C>
      static long test(...);

    public:
      constexpr static bool value = sizeof(test<T>(0)) == sizeof(char);
    };


    template <typename T, typename U,
        typename = std::enable_if_t<Comparable<T>::value and
            Comparable<U>::value>>
    bool compare(const T& lhs, const U& rhs)
    {
      return lhs == rhs;
    }


    template <typename T, typename U>
    bool compare (const T&, const U&)
    {
      return false;
    }
  }


  class Generic
  {
    class ContainerBase
    {
    public:
      virtual ~ContainerBase() = default;

      virtual bool operator==(const ContainerBase& container) const = 0;

      virtual std::unique_ptr<ContainerBase> clone() const = 0;

      virtual void* get_ptr() = 0;
      virtual const void* get_ptr() const = 0;
      virtual const std::type_index& get_type_index() const = 0;
    };

    template <typename T>
    class ValueContainer : public ContainerBase
    {
    public:
      ValueContainer(T value);

      bool operator==(const ContainerBase& container) const override;

      std::unique_ptr<ContainerBase> clone() const override;

      inline void* get_ptr() override;
      inline const void* get_ptr() const override;
      const std::type_index& get_type_index() const override { return type_; }

    private:
      std::type_index type_;
      T value_;
    };

    template <typename T>
    class PtrContainer : public ContainerBase
    {
    public:
      PtrContainer(T* ptr);
      PtrContainer(std::shared_ptr<T> ptr);

      bool operator==(const ContainerBase& container) const override;

      std::unique_ptr<ContainerBase> clone() const override;

      inline void* get_ptr() override;
      inline const void* get_ptr() const override;
      const std::type_index& get_type_index() const override { return type_; }

    private:
      std::type_index type_;
      std::shared_ptr<T> ptr_;
    };

  public:
    template <typename T,
        typename = std::enable_if_t<not std::is_pointer<T>::value, void>>
    Generic(T value);

    template <typename T>
    Generic(std::shared_ptr<T> ptr);

    template <typename T>
    Generic(T* ptr);

    Generic(const Generic& generic);
    Generic(Generic&& generic) noexcept;

    Generic& operator=(const Generic& generic);
    Generic& operator=(Generic&& generic) noexcept;

    bool operator==(const Generic& generic) const;

    template <typename T>
    inline const T& get() const;
    template <typename T>
    inline T& get();

    template <typename T, typename U>
    inline const U& get() const;
    template <typename T, typename U>
    inline U& get();

    const std::type_index& type() const { return container_->get_type_index(); }

    template <typename T>
    bool has_type() const { return std::type_index(typeid(T)) == type(); }

  private:
    template <typename T>
    void check_access() const
    {
      if (not has_type<T>()) {
        throw std::logic_error("Unable to get specified type from Generic!");
      }
    }

    std::unique_ptr<ContainerBase> container_;
  };


  template <typename T, typename>
  Generic::Generic(T value)
      : container_(new ValueContainer<T>(std::move(value)))
  {
  }


  template <typename T>
  Generic::Generic(std::shared_ptr<T> ptr)
      : container_(new PtrContainer<T>(std::move(ptr)))
  {
  }


  template <typename T>
  Generic::Generic(T* ptr)
      : container_(new PtrContainer<T>(ptr))
  {
  }


  template <typename T>
  const T& Generic::get() const
  {
    check_access<T>();
    return *reinterpret_cast<const T*>(container_->get_ptr());
  }


  template <typename T>
  T& Generic::get()
  {
    check_access<T>();
    return *reinterpret_cast<T*>(container_->get_ptr());
  }


  template <typename T, typename U>
  const U& Generic::get() const
  {
    check_access<T>();
    auto ptr = dynamic_cast<U*>(reinterpret_cast<T*>(container_->get_ptr()));

    if (ptr != nullptr) {
      return *ptr;
    }
    throw std::logic_error("Unable to cast type to that specified");
  }


  template <typename T, typename U>
  U& Generic::get()
  {
    check_access<T>();
    auto ptr = dynamic_cast<U*>(reinterpret_cast<T*>(container_->get_ptr()));

    if (ptr != nullptr) {
      return *ptr;
    }
    throw std::logic_error("Unable to cast type to that specified");
  }


  template <typename T>
  Generic::ValueContainer<T>::ValueContainer(T value)
      : type_(typeid(T)), value_(std::move(value))
  {
  }


  template <typename T>
  bool Generic::ValueContainer<T>::operator==(
      const ContainerBase& container) const
  {
    if (get_type_index() != container.get_type_index()) {
      return false;
    }

    return value_ == static_cast<const ValueContainer<T>&>(container).value_;
  }


  template <typename T>
  std::unique_ptr<Generic::ContainerBase>
  Generic::ValueContainer<T>::clone() const
  {
    return std::make_unique<ValueContainer>(value_);
  }


  template <typename T>
  void* Generic::ValueContainer<T>::get_ptr()
  {
    return reinterpret_cast<void*>(&value_);
  }


  template <typename T>
  const void* Generic::ValueContainer<T>::get_ptr() const
  {
    return reinterpret_cast<const void*>(&value_);
  }


  template <typename T>
  Generic::PtrContainer<T>::PtrContainer(T* ptr)
      : type_(typeid(T)), ptr_(ptr)
  {
  }


  template <typename T>
  Generic::PtrContainer<T>::PtrContainer(std::shared_ptr<T> ptr)
      : type_(typeid(T)), ptr_(std::move(ptr))
  {
  }


  template <typename T>
  bool Generic::PtrContainer<T>::operator==(
      const Generic::ContainerBase& container) const
  {
    using namespace detail;

    if (get_type_index() != container.get_type_index()) {
      return false;
    }

    return compare(*ptr_, *static_cast<const PtrContainer<T>&>(container).ptr_);
  }


  template <typename T>
  std::unique_ptr<Generic::ContainerBase>
  Generic::PtrContainer<T>::clone() const
  {
    return std::make_unique<PtrContainer>(ptr_);
  }


  template <typename T>
  void* Generic::PtrContainer<T>::get_ptr()
  {
    return reinterpret_cast<void*>(ptr_.get());
  }


  template <typename T>
  const void* Generic::PtrContainer<T>::get_ptr() const
  {
    return reinterpret_cast<void*>(ptr_.get());
  }
}

#endif //LOXX_GENERIC_HPP