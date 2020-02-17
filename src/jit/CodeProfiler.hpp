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
 * Created by Matt Spraggs on 13/10/2019.
 */

#ifndef LOXX_JIT_CODEPROFILER_HPP
#define LOXX_JIT_CODEPROFILER_HPP

#include <tuple>
#include <vector>

#include "../CodeObject.hpp"
#include "../Object.hpp"
#include "../Stack.hpp"
#include "../Value.hpp"

#include "AssemblyFunction.hpp"
#include "Compiler.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    class CodeProfiler
    {
    public:
      CodeProfiler(
          Compiler& compiler, const bool debug,
          const std::size_t block_count_threshold)
          : debug_(debug), is_recording_(false),
            block_count_threshold_(block_count_threshold)
      {
      }

      void count_basic_block(const CodeObject::InsPtr ip);

      void skip_current_block();

      void record_instruction(
          const CodeObject::InsPtr ip, const RuntimeContext context);

      bool is_recording() const { return is_recording_; }

    private:
      template <typename T>
      using InsPtrHashTable = HashTable<CodeObject::InsPtr, T, InsPtrHasher>;

      enum class OperandLocation
      {
        CONSTANT,
        LOCAL,
        UPVALUE,
        GLOBAL,
      };

      using OperandIndex = std::pair<OperandLocation, std::size_t>;

      struct OperandIndexHasher
      {
        bool operator() (const OperandIndex& value) const;
      };

      void start_recording();
      void finalise_ir();

      bool debug_, is_recording_;
      std::size_t block_count_threshold_;
      CodeObject::InsPtr current_block_head_;

      HashSet<CodeObject::InsPtr, InsPtrHasher> ignored_blocks_;
      InsPtrHashTable<std::size_t> block_counts_;
      InsPtrHashTable<std::size_t> ssa_ir_map_;
      std::vector<SSAInstruction<3>> ssa_ir_;
      HashTable<const Value*, Operand> exit_assignments_;
      std::vector<SSAInstruction<3>> entry_code_;
      std::vector<std::pair<CodeObject::InsPtr, std::size_t>> jump_targets_;
      Stack<Operand, max_stack_size> op_stack_;
      HashTable<OperandIndex, Operand, OperandIndexHasher> operand_cache_;
    };
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
