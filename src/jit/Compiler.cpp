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
 * Created by Matt Spraggs on 22/11/2019.
 */

#include <iostream>

#include "../Instruction.hpp"
#include "../logging.hpp"
#include "../utils.hpp"
#include "../VirtualMachine.hpp"

#include "AssemblerX86.hpp"
#include "CodeProfiler.hpp"
#include "Compiler.hpp"
#include "logging.hpp"
#include "Optimiser.hpp"
#include "RegisterAllocator.hpp"


namespace loxx
{
  namespace jit
  {
    namespace detail
    {
      template <typename T>
      T read_integer(CodeObject::InsPtr& ip);
    }


    void compile_trace(Trace& trace, const bool debug)
    {
      // optimise(trace);

#ifndef NDEBUG
      if (debug) {
        std::cout << "=== Generated SSA ===\n";
        print_trace(trace);
      }
#endif

      RegisterAllocator reg_alloc(debug, get_allocatable_registers<platform>());
      reg_alloc.allocate(trace);

#ifndef NDEBUG
      if (debug) {
        print_allocation_map(trace.allocation_map);
      }
#endif
      Assembler<platform> assembler(trace);
      assembler.assemble();
    }
  }
}
