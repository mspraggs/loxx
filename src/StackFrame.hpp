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


namespace loxx
{
  class StackFrame
  {
  public:
    StackFrame(const std::size_t prev_ip, const std::size_t num_locals)
        : prev_ip_(prev_ip), locals_(num_locals)
    {
      locals_.reserve(4096);
    }

    void reserve_local_space(const std::size_t size);

    std::size_t prev_ip() const { return prev_ip_; }

    ByteCodeArg make_local(const std::string& lexeme,
                           const StackVar& value);

    const StackVar& get_local(const std::size_t index) const
    { return locals_[index]; }

    void set_local(const std::size_t index, StackVar value)
    { locals_[index] = std::move(value); }

    bool is_declared(const std::size_t index) const
    { return is_declared_[index]; }

  private:
    std::size_t prev_ip_;
    std::unordered_map<std::string, ByteCodeArg> local_map_;
    std::vector<StackVar> locals_;
    std::vector<bool> is_declared_;
  };
}

#endif //LOXX_STACKFRAME_HPP
