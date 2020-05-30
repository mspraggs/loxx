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

#include "JITError.hpp"
#include "Snapshot.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    std::size_t create_snapshot(
        Trace& trace, const CodeObject::InsPtr ip,
        const TaggedStack<std::size_t, StackTag, max_stack_size>& stack)
    {
      const auto exit_num = trace.snaps.size();

      if (exit_num == max_num_snapshots) {
        throw JITError("max num snapshots reached");
      }

      auto stack_map_it = [&] {
        if (exit_num == 0) {
          return trace.vstack_ir_map.begin();
        }
        return trace.snaps.back().stack_map_end;
      } ();
      const auto stack_map_begin = stack_map_it;

      for (std::size_t i = 0; i < stack.size(); ++i) {
        if (stack[i].has_tag(StackTag::WRITTEN)) {
          *stack_map_it++ = VStackIRPairing{i, stack[i].value};
        }
      }

      trace.snaps.emplace_back(
          Snapshot{
            .ir_ref = trace.ir_buffer.size(),
            .next_ip = ip,
            .stack_map_begin = stack_map_begin,
            .stack_map_end = stack_map_it,
          }
      );

      return exit_num;
    }
  }
}
