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
 * Created by Matt Spraggs on 16/11/17.
 */

#ifndef LOXX_STACK_HPP
#define LOXX_STACK_HPP

#include <array>
#include <stdexcept>
#include <vector>


namespace loxx
{
  template <typename T, std::size_t N = 4096>
  class Stack
  {
  public:
    explicit Stack() : top_(0) {}

    T* data() { return stack_.data(); }
    const T* data() const { return stack_.data(); }
    T& top(const std::size_t depth = 0)
    { return stack_[top_ - 1 - depth]; }
    const T& top(const std::size_t depth = 0) const
    { return stack_[top_ - 1 - depth]; }
    T& get(const std::size_t idx) { return stack_[idx]; }
    const T& get(const std::size_t idx) const { return stack_[idx]; }
    void push(T value);
    T pop();

    std::size_t size() const { return top_; }

  private:
    std::size_t top_;
    std::array<T, N> stack_;
  };


  template <typename T, std::size_t N>
  void Stack<T, N>::push(T value)
  {
    if (top_ == N) {
      throw std::overflow_error("Stack overflow!");
    }
    stack_[top_++] = std::move(value);
  }


  template <typename T, std::size_t N>
  T Stack<T, N>::pop()
  {
    return stack_[--top_];
  }
}

#endif //LOXX_STACK_HPP
