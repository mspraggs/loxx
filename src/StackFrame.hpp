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
    StackFrame(const std::size_t prev_ip, Value& slots_base)
        : prev_ip_(prev_ip), slots_(&slots_base)
    {}

    std::size_t prev_ip() const { return prev_ip_; }
    const Value* slots_base() const { return slots_ - 1; }

    const Value& slot(const std::size_t i) const { return slots_[i]; }
    Value& slot(const std::size_t i) { return slots_[i]; }

  private:
    std::size_t prev_ip_;
    Value* slots_;
  };
}

#endif //LOXX_STACKFRAME_HPP
