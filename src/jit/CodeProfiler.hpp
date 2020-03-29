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

#include "AssemblyWrapper.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    class CodeProfiler
    {
    public:
      CodeProfiler(
          TraceCache& trace_cache, const std::size_t block_count_threshold)
          : is_recording_(false), block_count_threshold_(block_count_threshold),
            trace_cache_(&trace_cache)
      {
      }

      void handle_basic_block_head(const CodeObject::InsPtr ip);

      void skip_current_block();

      void record_instruction(
          const CodeObject::InsPtr ip, const RuntimeContext context);

      bool is_recording() const { return is_recording_; }

    private:
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
      void peel_loop(const CodeObject::InsPtr ip);
      VRegHashTable<VirtualRegister> CodeProfiler::build_loop_vreg_map() const;
      void emit_loop_moves(const VRegHashTable<VirtualRegister>& loop_vreg_map);
      void emit_loop(
          const CodeObject::InsPtr ip, const std::size_t ir_offset);
      void patch_jumps();
      void emit_exit_assignments();

      bool is_recording_;
      std::size_t block_count_threshold_;
      CodeObject::InsPtr current_block_head_;

      TraceCache* trace_cache_;

      HashSet<CodeObject::InsPtr, CodeObject::InsPtrHasher> ignored_blocks_;
      CodeObject::InsPtrHashTable<std::size_t> block_counts_;
      CodeObject::InsPtrHashTable<std::size_t> ssa_ir_map_;
      SSABuffer<3> ssa_ir_;
      HashTable<const Value*, Operand> exit_assignments_;
      std::vector<std::pair<CodeObject::InsPtr, std::size_t>> jump_targets_;
      Stack<Operand, max_stack_size> op_stack_;
      HashTable<const Value*, Operand> operand_cache_;
      std::vector<CodeObject::InsPtr> recorded_instructions_;
    };
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
