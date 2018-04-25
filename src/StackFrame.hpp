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
 * Created by Matt Spraggs on 12/03/2018.
 */

#ifndef LOXX_STACKFRAME_HPP
#define LOXX_STACKFRAME_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "globals.hpp"
#include "Value.hpp"


namespace loxx
{
  class StackFrame
  {
  public:
    StackFrame(const std::size_t prev_ip, const std::size_t prev_stack_size,
               Value& slots_base, raw_ptr<ClosureObject> closure)
        : StackFrame(prev_ip, prev_stack_size, &slots_base, closure)
    {}
    StackFrame(const std::size_t prev_ip, const std::size_t prev_stack_size,
               raw_ptr<Value> slots_base, raw_ptr<ClosureObject> closure)
        : prev_ip_(prev_ip), prev_stack_size_(prev_stack_size),
          slots_(slots_base), closure_(closure)
    {}

    std::size_t prev_ip() const { return prev_ip_; }
    std::size_t prev_stack_size() const { return prev_stack_size_; }

    const Value& slot(const std::size_t i) const { return slots_[i]; }
    Value& slot(const std::size_t i) { return slots_[i]; }

    raw_ptr<const ClosureObject> closure() const { return closure_; }
    raw_ptr<ClosureObject> closure() { return closure_; }

  private:
    std::size_t prev_ip_, prev_stack_size_;
    raw_ptr<Value> slots_;
    raw_ptr<ClosureObject> closure_;
  };
}

#endif //LOXX_STACKFRAME_HPP
