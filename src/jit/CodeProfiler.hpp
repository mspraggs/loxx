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

#include "SSAGenerator.hpp"
#include "SSAInstruction.hpp"


namespace loxx
{
  struct RuntimeContext;


  namespace jit
  {
    struct BlockInfo
    {
      const CodeObject* code;
      CodeObject::InsPtr begin;
      CodeObject::InsPtr end;
    };


    class CodeProfiler
    {
    public:
      CodeProfiler(const bool debug, const std::size_t block_count_threshold)
          : debug_(debug), block_boundary_flagged_(true),
            block_count_threshold_(block_count_threshold), hot_block_(nullptr),
            ssa_generator_(debug)
      {
      }

      void count_basic_block(
          const CodeObject::InsPtr ip,
          const RuntimeContext& context);

      template <typename... Ts>
      void profile_instruction(
          const CodeObject::InsPtr ip, const Ts&... ops);

      void profile_instruction(
          const CodeObject::InsPtr ip,
          const Value* start, const std::size_t size);

      void flag_block_boundary(const CodeObject::InsPtr ip);

      bool is_profiling_instructions() const { return hot_block_; }
      bool block_boundary_flagged() const { return block_boundary_flagged_; }

    private:

      struct BlockInfoHasher
      {
        std::size_t operator() (const BlockInfo& value) const;

        InsPtrHasher ip_hasher;
      };

      struct BlockInfoCompare
      {
        bool operator() (const BlockInfo& info1, const BlockInfo& info2) const;
      };

      bool debug_, block_boundary_flagged_;
      std::size_t block_count_threshold_;
      BlockInfo* hot_block_;

      HashTable<BlockInfo, std::size_t, BlockInfoHasher, BlockInfoCompare>
          block_counts_;
      SSAGenerator ssa_generator_;
    };


    template <typename... Ts>
    void CodeProfiler::profile_instruction(
        const CodeObject::InsPtr ip, const Ts&... ops)
    {
      if (not hot_block_) {
        return;
      }
    }
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
