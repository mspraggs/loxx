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

#include "StackFrame.hpp"


namespace loxx
{
  void StackFrame::reserve_local_space(const std::size_t size)
  {
    if (locals_.size() < size) {
      locals_.resize(size);
    }
  }


  UByteCodeArg StackFrame::make_local(const std::string& lexeme,
                                     const StackVar& value)
  {
    if (local_map_.count(lexeme) != 0) {
      return local_map_[lexeme];
    }

    const auto index = static_cast<UByteCodeArg>(locals_.size());

    locals_.push_back(value);
    local_map_[lexeme] = index;

    return index;
  }
}

