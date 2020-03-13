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
#include "SSAInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    class TraceCache
    {
    public:
      template <typename T>
      using InsPtrHashTable = CodeObject::InsPtrHashTable<T>;
      using IRCacheElem = std::pair<CodeObject::InsPtr, SSABuffer<3>>;
      using RecordedInstructions = std::vector<CodeObject::InsPtr>;

      void add_ssa_ir(
          const CodeObject::InsPtr begin, const CodeObject::InsPtr end,
          RecordedInstructions instructions, SSABuffer<3> ssa_ir);
      auto get_ssa_ir(const CodeObject::InsPtr ip) const
          -> const InsPtrHashTable<IRCacheElem>::Elem&;
      auto get_ssa_ir(const CodeObject::InsPtr ip)
          -> InsPtrHashTable<IRCacheElem>::Elem&;
      auto get_recorded_instructions(const CodeObject::InsPtr ip) const
          -> const RecordedInstructions&;

      void add_assembly(const CodeObject::InsPtr ip, AssemblyWrapper assembly);
      auto get_assembly(const CodeObject::InsPtr ip) const
          -> const CodeObject::InsPtrHashTable<AssemblyWrapper>::Elem&;

    private:
      InsPtrHashTable<IRCacheElem> ir_cache_;
      InsPtrHashTable<RecordedInstructions> recorded_ips_;
      InsPtrHashTable<AssemblyWrapper> assembly_cache_;
    };
  }
}

#endif // LOXX_JIT_TRACECACHE_HPP
