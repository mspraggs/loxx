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
#include <typeindex>
#include <typeinfo>


namespace loxx
{
  class Generic
  {
    class ContainerBase
    {
    public:
      virtual ~ContainerBase() = default;

      virtual void* get_ptr() = 0;
      virtual const void* get_ptr() const = 0;
    };

    template <typename T>
    class Container : public ContainerBase
    {
    public:
      Container(T value) : value_(std::move(value)) {}

      void* get_ptr() override { return reinterpret_cast<void*>(&value_); }
      const void* get_ptr() const override
      { return reinterpret_cast<const void*>(&value_); }

    private:
      T value_;
    };

  public:
    template <typename T>
    Generic(T value)
        : type_(typeid(T)), container_(new Container<T>(std::move(value)))
    {}

    Generic(Generic&& generic) noexcept
        : type_(generic.type_), container_(std::move(generic.container_))
    {}

    Generic& operator=(Generic&& generic) noexcept
    {
      type_ = generic.type_;
      container_ = std::move(generic.container_);

      return *this;
    }

    template <typename T>
    const T& get() const
    {
      check_access<T>();
      return *reinterpret_cast<const T*>(container_->get_ptr());
    }

    template <typename T>
    T& get()
    {
      check_access<T>();
      return *reinterpret_cast<T*>(container_->get_ptr());
    }

  private:
    template <typename T>
    void check_access() const
    {
      if (std::type_index(typeid(T)) != type_) {
        throw std::logic_error("Unable to get specified type from Generic!");
      }
    }

    std::type_index type_;
    std::unique_ptr<ContainerBase> container_;
  };
}

#endif //LOXX_GENERIC_HPP
