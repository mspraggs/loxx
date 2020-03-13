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

#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    void TraceCache::add_ssa_ir(
        const CodeObject::InsPtr begin, const CodeObject::InsPtr end,
        RecordedInstructions instructions, SSABuffer<3> ssa_ir)
    {
      ir_cache_[begin] = std::make_pair(end, std::move(ssa_ir));
      recorded_ips_[begin] = std::move(instructions);
    }


    auto TraceCache::get_ssa_ir(const CodeObject::InsPtr ip) const
        -> const InsPtrHashTable<IRCacheElem>::Elem&
    {
      return ir_cache_.get(ip);
    }


    auto TraceCache::get_ssa_ir(const CodeObject::InsPtr ip)
        -> InsPtrHashTable<IRCacheElem>::Elem&
    {
      return ir_cache_.get(ip);
    }


    auto TraceCache::get_recorded_instructions(
        const CodeObject::InsPtr ip) const -> const RecordedInstructions&
    {
      return recorded_ips_.at(ip);
    }


    void TraceCache::add_assembly(
        const CodeObject::InsPtr ip, AssemblyWrapper assembly)
    {
      assembly.lock();
      assembly_cache_[ip] = std::move(assembly);
    }


    auto TraceCache::get_assembly(const CodeObject::InsPtr ip) const
        -> const CodeObject::InsPtrHashTable<AssemblyWrapper>::Elem&
    {
      return assembly_cache_.get(ip);
    }
  }
}
