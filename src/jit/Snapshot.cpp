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
 * Created by Matt Spraggs on 07/05/2020.
 */

#include "Snapshot.hpp"


namespace loxx
{
  namespace jit
  {
    std::vector<std::pair<std::size_t, VStackElem>> compress_stack(
        const TaggedStack<std::size_t, StackTag, max_stack_size>& stack)
    {
      using Elem = TaggedStack<std::size_t, StackTag, max_stack_size>::Elem;

      std::vector<std::pair<std::size_t, Elem>> compressed_stack;
      compressed_stack.reserve(stack.size());

      for (std::size_t i = 0; i < stack.size(); ++i) {
        if (stack[i].tags != 0) {
          compressed_stack.emplace_back(i, stack[i]);
        }
      }

      return compressed_stack;
    }
  }
}
