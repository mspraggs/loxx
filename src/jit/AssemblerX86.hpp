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
 * Created by Matt Spraggs on 21/12/2018.
 */

#ifndef LOXX_JIT_ASSEMBLERX86_HPP
#define LOXX_JIT_ASSEMBLERX86_HPP

#include "Assembler.hpp"


namespace loxx
{
  namespace jit
  {
    template <>
    class Assembler<RegisterX86>
    {
    public:
      AssemblyFunction assemble(
          const std::vector<SSAInstruction<2>>& ssa_ir,
          const AllocationMap<RegisterX86>& allocation_map);

    private:
      void add_return();
      void add_push(const RegisterX86 src);
      void add_pop(const RegisterX86 dst);
      void add_move(const RegisterX86 dst, const RegisterX86 src);

      AssemblyFunction func_; 
    };
  }
}

#endif // LOXX_JIT_ASSEMBLERX86_HPP
