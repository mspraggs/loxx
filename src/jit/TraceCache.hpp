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
 * Created by Matt Spraggs on 18/02/2020.
 */

#ifndef LOXX_JIT_TRACECACHE_HPP
#define LOXX_JIT_TRACECACHE_HPP

#include <tuple>
#include <vector>

#include "../CodeObject.hpp"

#include "AssemblyWrapper.hpp"
#include "RegisterAllocator.hpp"
#include "IRInstruction.hpp"
#include "Snapshot.hpp"


namespace loxx
{
  namespace jit
  {
    struct Trace
    {
      enum class State {
        NEW,
        IR_COMPLETE,
        ASSEMBLY_COMPLETE,
      };
      State state;
      CodeObject::InsPtr init_ip;
      CodeObject::InsPtr next_ip;
      IRBuffer<3> ir_buffer;
      CodeObject::InsPtrHashTable<std::size_t> ir_map;
      std::vector<CodeObject::InsPtr> recorded_instructions;
      std::vector<Snapshot> snaps;
      AssemblyWrapper assembly;
      AllocationMap<Register> allocation_map;
      Trace* chained_trace;
      const Value* stack_base;
      const CodeObject* code_object;
    };


    class TraceCache
    {
    public:
      Trace* active_trace() { return active_.get(); }
      const Trace* active_trace() const { return active_.get(); }

      Trace* get_trace(const CodeObject::InsPtr ip);
      const Trace* get_trace(const CodeObject::InsPtr ip) const;

      void make_new_trace();
      void store_active_trace();

    private:
      std::unique_ptr<Trace> active_;
      CodeObject::InsPtrHashTable<Trace*> trace_map_;
      std::vector<std::unique_ptr<Trace>> traces_;
    };
  }
}

#endif // LOXX_JIT_TRACECACHE_HPP
