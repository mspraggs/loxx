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
            trace_(nullptr), trace_cache_(&trace_cache)
      {
      }

      void handle_basic_block_head(const CodeObject::InsPtr ip);

      void skip_current_block();

      void record_instruction(
          const CodeObject::InsPtr ip, const RuntimeContext context);

      bool is_recording() const { return is_recording_; }

    private:
      void start_recording(const CodeObject::InsPtr ip);
      bool instruction_ends_current_block(const CodeObject::InsPtr ip) const;
      void peel_loop();
      VRegHashTable<VirtualRegister> build_loop_vreg_map() const;
      void emit_loop_moves(const VRegHashTable<VirtualRegister>& loop_vreg_map);
      void emit_loop();
      void patch_jumps();
      void emit_exit_assignments();

      bool virtual_registers_are_floats(
          const VirtualRegister& first,
          const VirtualRegister& second) const;

      template <typename... Args>
      void emit_ir(const Operator op, Args&&... args);

      bool is_recording_;
      std::size_t block_count_threshold_;
      CodeObject::InsPtr current_block_head_;

      Trace* trace_;
      TraceCache* trace_cache_;

      HashSet<CodeObject::InsPtr, CodeObject::InsPtrHasher> ignored_blocks_;
      CodeObject::InsPtrHashTable<std::size_t> block_counts_;
      HashTable<const Value*, VirtualRegister> exit_assignments_;
      std::vector<std::pair<CodeObject::InsPtr, std::size_t>> jump_targets_;
      Stack<VirtualRegister, max_stack_size> vreg_stack_;
      HashTable<const Value*, VirtualRegister> vreg_cache_;
    };


    template <typename... Args>
    void CodeProfiler::emit_ir(const Operator op, Args&&... args)
    {
      trace_->ir_buffer.emplace_back(op, Operand(args)...);
    }
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
