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
 * Created by Matt Spraggs on 14/04/2020.
 */

#ifndef LOXX_JIT_SNAPSHOT_HPP
#define LOXX_JIT_SNAPSHOT_HPP

#include <vector>

#include "../CodeObject.hpp"

#include "TaggedStack.hpp"


namespace loxx
{
  namespace jit
  {
    enum class StackTag
    {
      EMPTY = 0,
      CACHED = 1,
      WRITTEN = 2,
    };


    using VStackElem = TaggedElem<std::size_t, StackTag>;


    struct Snapshot
    {
      std::size_t ir_ref;
      CodeObject::InsPtr next_ip;
      std::vector<std::pair<std::size_t, VStackElem>> stack_ir_map;
    };


    std::vector<std::pair<std::size_t, VStackElem>> compress_stack(
        const TaggedStack<std::size_t, StackTag, max_stack_size>& stack);
  }
}

#endif // LOXX_JIT_SNAPSHOT_HPP
