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
 * Created by Matt Spraggs on 26/04/2020.
 */

#ifndef LOXX_JIT_OPTIONALSTACK_HPP
#define LOXX_JIT_OPTIONALSTACK_HPP

#include <array>

#include "../Optional.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    template <typename T, std::size_t N>
    class OptionalStack
    {
    public:
      OptionalStack() : top_(0) {}

      void set(const std::size_t i, const T& value);
      const Optional<T>& get(const std::size_t i) const;
      template <typename... Args>
      void emplace(Args&&... values);
      void push(const T& value);
      T pop();
      const T& top(const std::size_t depth = 0) const
      { return *stack_[top_ - depth - 1]; }
      void reset_slot(const std::size_t i) { stack_[i].reset(); }

      std::size_t size() const { return top_; }
      void resize(const std::size_t size) { top_ = size; }

    private:
      std::size_t top_;
      std::array<Optional<T>, N> stack_;
    };


    template <typename T, std::size_t N>
    void OptionalStack<T, N>::set(const std::size_t i, const T& value)
    {
      stack_[i] = value;

      if (i >= top_) {
        top_ = i + 1;
      }
    }


    template <typename T, std::size_t N>
    const Optional<T>& OptionalStack<T, N>::get(const std::size_t i) const
    {
      return stack_[i];
    }


    template <typename T, std::size_t N>
    template <typename... Args>
    void OptionalStack<T, N>::emplace(Args&&... values)
    {
      stack_[top_++] = T(std::forward<Args>(values)...);
    }


    template <typename T, std::size_t N>
    void OptionalStack<T, N>::push(const T& value)
    {
      stack_[top_++] = value;
    }


    template <typename T, std::size_t N>
    T OptionalStack<T, N>::pop()
    {
      const auto ret = *stack_[--top_];
      stack_[top_].reset();
      return ret;
    }
  }
}

#endif // LOXX_JIT_OPTIONALSTACK_HPP
