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
#include "TaggedStack.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    class CodeProfiler
    {
      enum class Tag
      {
        EMPTY = 0,
        CACHED = 1,
        WRITTEN = 2,
      };
    public:
      CodeProfiler(
          TraceCache& trace_cache, const std::size_t block_count_threshold)
          : is_recording_(false), block_count_threshold_(block_count_threshold),
            trace_(nullptr), trace_cache_(&trace_cache),
            stack_({Tag::CACHED})
      {
      }

      void handle_basic_block_head(
          const CodeObject::InsPtr ip, const RuntimeContext context);

      void skip_current_block();

      void record_instruction(
          const CodeObject::InsPtr ip, const RuntimeContext context);

      bool is_recording() const { return is_recording_; }

    private:
      void start_recording(
          const CodeObject::InsPtr ip, const RuntimeContext context);
      bool instruction_ends_current_block(const CodeObject::InsPtr ip) const;
      void peel_loop();
      HashTable<std::size_t, std::size_t> build_loop_ir_ref_map() const;
      void emit_loop_moves(
          const HashTable<std::size_t, std::size_t>& loop_vreg_map);
      void emit_loop();
      void patch_jumps();
      void emit_exit_assignments();

      bool virtual_registers_are_floats(
          const std::size_t first, const std::size_t second) const;

      template <typename... Args>
      std::size_t emit_ir(
          const Operator op, const ValueType type, Args&&... args);

      bool is_recording_;
      std::size_t block_count_threshold_;
      CodeObject::InsPtr current_block_head_;

      Trace* trace_;
      TraceCache* trace_cache_;

      HashSet<CodeObject::InsPtr, CodeObject::InsPtrHasher> ignored_blocks_;
      CodeObject::InsPtrHashTable<std::size_t> block_counts_;
      HashTable<std::size_t, std::size_t> exit_assignments_;
      std::vector<std::pair<CodeObject::InsPtr, std::size_t>> jump_targets_;
      TaggedStack<std::size_t, Tag, max_stack_size> stack_;
    };


    template <typename... Args>
    std::size_t CodeProfiler::emit_ir(
        const Operator op, const ValueType type, Args&&... args)
    {
      const auto ret = trace_->ir_buffer.size();
      trace_->ir_buffer.emplace_back(op, type, Operand(args)...);
      return ret;
    }
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
