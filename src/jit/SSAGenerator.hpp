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
 * Created by Matt Spraggs on 14/11/2019.
 */

#ifndef LOXX_JIT_SSAGENERATOR_HPP
#define LOXX_JIT_SSAGENERATOR_HPP

#include "../CodeObject.hpp"
#include "../Object.hpp"

#include "SSAInstruction.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    class InstructionData;


    using InstructionDataRepo =
        HashTable<CodeObject::InsPtr, InstructionData, InsPtrHasher>;


    class SSAGenerator
    {
    public:
      explicit SSAGenerator(const bool debug) : debug_(debug) {}

      void build_context(const RuntimeContext& context);

      std::vector<SSAInstruction> generate(
          const CodeObject::InsPtr begin, const CodeObject::InsPtr end);

    private:
      bool debug_;
      Stack<Operand, max_stack_size> op_stack_;
      std::vector<Operand> constants_;
      std::vector<Operand> upvalues_;
      StringHashTable<Operand> globals_;
    };
  }
}

#endif // LOXX_JIT_SSAGENERATOR_HPP