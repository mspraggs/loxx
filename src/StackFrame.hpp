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

  private:
    std::unordered_map<std::string, ByteCodeArg> local_map_;
    std::vector<StackVar> locals_;
  };
}

#endif //LOXX_STACKFRAME_HPP
