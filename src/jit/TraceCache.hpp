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

#include "SSAInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    class TraceCache
    {
      using SSAData = std::pair<CodeObject::InsPtr, SSABuffer<3>>;

    public:
      void add_ssa_ir(
          const CodeObject::InsPtr begin, const CodeObject::InsPtr end,
          SSABuffer<3> ssa_ir);
      auto get_ssa_ir(const CodeObject::InsPtr ip) const
          -> const CodeObject::InsPtrHashTable<SSAData>::Elem&;

    private:
      CodeObject::InsPtrHashTable<SSAData> ir_cache_;
    };
  }
}

#endif // LOXX_JIT_TRACECACHE_HPP
