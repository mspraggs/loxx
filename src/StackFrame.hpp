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

#include "CodeObject.hpp"
#include "globals.hpp"
#include "Object.hpp"
#include "Value.hpp"


namespace loxx
{
  class StackFrame
  {
  public:
    StackFrame() = default;
    StackFrame(
        const CodeObject::InsPtr prev_ip, const CodeObject* prev_code_object,
        Value* slots_base, ClosureObject* closure)
        : prev_ip_(prev_ip), prev_code_object_(prev_code_object),
          slots_(slots_base), closure_(closure)
    {}

    CodeObject::InsPtr prev_ip() const { return prev_ip_; }
    const CodeObject* prev_code_object() const
    { return prev_code_object_; }

    const Value& slot(const std::size_t i) const { return slots_[i]; }
    Value& slot(const std::size_t i) { return slots_[i]; }

    const ClosureObject* closure() const { return closure_; }
    ClosureObject* closure() { return closure_; }

  private:
    CodeObject::InsPtr prev_ip_;
    const CodeObject* prev_code_object_;
    Value* slots_;
    ClosureObject* closure_;
  };
}

#endif //LOXX_STACKFRAME_HPP
