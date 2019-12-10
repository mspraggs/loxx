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

#include "CodeProfiler.hpp"
#include "Compiler.hpp"
#include "logging.hpp"
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


    void Compiler::build_context(const RuntimeContext& context)
    {
      ssa_generator_.build_context(context);
    }


    void Compiler::compile(
        const CodeObject::InsPtr begin, const CodeObject::InsPtr end)
    {
      ssa_ir_ = ssa_generator_.generate(begin, end);

  #ifndef NDEBUG
      if (debug_) {
        std::cout << "=== Generated SSA ===\n";
        print_ssa_instructions(ssa_ir_);
      }
  #endif

      RegisterAllocator reg_alloc(debug_, get_platform_registers<Register>());
      reg_alloc.allocate(ssa_ir_);
    }
  }
}
