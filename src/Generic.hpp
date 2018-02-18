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
  class bad_generic_cast : public std::bad_cast
  {
  };


  class Generic
  {
    struct ContainerBase
    {
      virtual ~ContainerBase() = default;

      virtual bool operator==(const ContainerBase& container) const = 0;

      virtual ContainerBase* clone() const = 0;

      virtual const std::type_info& type() const { return typeid(void); };
    };

    template <typename T>
    struct Container : public ContainerBase
    {
      Container(T value);

      bool operator==(const ContainerBase& container) const override;

      ContainerBase* clone() const override;

      const std::type_info& type() const override { return typeid(T); }

      T value;
    };

  public:
    Generic() = default;

    template <typename T>
    Generic(T value);

    Generic(const Generic& generic);
    Generic(Generic&& generic) noexcept;

    Generic& operator=(const Generic& generic);
    Generic& operator=(Generic&& generic) noexcept;

    template <typename T>
    Generic& operator=(const T& value);
    template <typename T>
    Generic& operator=(T&& value);

    bool operator==(const Generic& generic) const;

    const std::type_info& type() const { return container_->type(); }

    bool empty() const { return container_ == nullptr; }

    template <typename T>
    bool has_type() const
    { return typeid(T) == container_->type(); }

  private:
    template <typename T>
    friend T* generic_cast(Generic*);

    std::unique_ptr<ContainerBase> container_;
  };


  template <typename T>
  Generic::Generic(T value) : container_(new Container<T>(std::move(value)))
  {
  }


  template<typename T>
  Generic& Generic::operator=(const T& value)
  {
    if (has_type<T>()) {
      static_cast<Container<T>*>(container_.get())->value = value;
    }
    else {
      container_.reset(new Container<T>(value));
    }
    return *this;
  }


  template<typename T>
  Generic& Generic::operator=(T&& value)
  {
    if (has_type<T>()) {
      static_cast<Container<T>*>(container_.get())->value = value;
    }
    else {
      container_.reset(new Container<T>(value));
    }
    return *this;
  }


  template <typename T>
  Generic::Container<T>::Container(T value)
      : value(std::move(value))
  {
  }


  template <typename T>
  bool Generic::Container<T>::operator==(const ContainerBase& container) const
  {
    if (type() != container.type()) {
      return false;
    }

    return value == static_cast<const Container<T>&>(container).value;
  }


  template <typename T>
  Generic::ContainerBase* Generic::Container<T>::clone() const
  {
    return new Container(value);
  }


  template <typename T>
  T* generic_cast(Generic* generic)
  {
    auto ptr = generic->container_.get();
    return generic->has_type<T>() ?
           &static_cast<Generic::Container<T>*>(ptr)->value : nullptr;
  }


  template <typename T>
  const T* generic_cast(const Generic* generic)
  {
    return const_cast<const T*>(generic_cast<T>(const_cast<Generic*>(generic)));
  }


  template <typename T>
  T& generic_cast(Generic& generic)
  {
    auto ptr = generic_cast<T>(&generic);

    if (ptr != nullptr) {
      return *ptr;
    }

    throw std::bad_cast();
  }


  template <typename T>
  const T& generic_cast(const Generic& generic)
  {
    const auto ptr = generic_cast<T>(&generic);

    if (ptr != nullptr) {
      return *ptr;
    }

    throw bad_generic_cast();
  }
}

#endif //LOXX_GENERIC_HPP