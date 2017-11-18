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

#include <vector>


namespace loxx
{
  template <typename T>
  class Stack
  {
  public:
    Stack(const std::size_t reserved_size = 4096)
    {
      stack_.reserve(reserved_size);
    }

    const T& top() const { return stack_.back(); }
    void push(T value);
    T pop();

    std::size_t size() const { return stack_.size(); }

  private:
    std::vector<T> stack_;
  };


  template <typename T>
  void Stack<T>::push(T value)
  {
    stack_.emplace_back(std::move(value));
  }


  template <typename T>
  T Stack<T>::pop()
  {
    if (stack_.size() < 1) {
      throw std::logic_error("Cannot pop item off empty stack!");
    }

    const auto value = std::move(stack_.back());
    stack_.pop_back();
    return value;
  }
}

#endif //LOXX_STACK_HPP
