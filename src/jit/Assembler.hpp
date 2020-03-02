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
 * Created by Matt Spraggs on 15/12/2019.
 */

#ifndef LOXX_JIT_ASSEMBLER_HPP
#define LOXX_JIT_ASSEMBLER_HPP

#include "AssemblyWrapper.hpp"
#include "RegisterAllocator.hpp"
#include "SSAInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    template <Platform P>
    class Assembler
    {
    public:
      using SSAInstruction = SSAInstruction<3>;

      AssemblyWrapper assemble(
          const std::vector<SSAInstruction>& ssa_ir,
          const AllocationMap<RegType<P>>& allocation_map);
    };
  }
}

#endif // LOXX_JIT_ASSEMBLER_HPP
