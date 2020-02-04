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
    struct BlockInfo
    {
      const CodeObject* code;
      CodeObject::InsPtr begin;
      CodeObject::InsPtr end;
    };


    class CodeProfiler
    {
    public:
      CodeProfiler(
          Compiler& compiler, const bool debug,
          const std::size_t block_count_threshold)
          : debug_(debug), skip_current_block_(false),
            block_count_threshold_(block_count_threshold), compiler_(&compiler)
      {
      }

      CodeObject::InsPtr enter_basic_block(
          const CodeObject::InsPtr ip,
          const RuntimeContext& context);

      void exit_basic_block(const CodeObject::InsPtr ip);

      void skip_current_block();

      bool is_recording() const { return is_recording_; }

    private:
      template <typename T>
      using InsPtrHashTable = HashTable<CodeObject::InsPtr, T, InsPtrHasher>;
      using CompilationResult = std::pair<AssemblyFunction, CodeObject::InsPtr>;

      struct BlockInfoHasher
      {
        std::size_t operator() (const BlockInfo& value) const;

        InsPtrHasher ip_hasher;
      };

      struct BlockInfoCompare
      {
        bool operator() (const BlockInfo& info1, const BlockInfo& info2) const;
      };

      bool debug_, skip_current_block_, is_recording_;
      std::size_t block_count_threshold_;
      CodeObject::InsPtr current_block_;
      std::unique_ptr<BlockInfo> hot_block_;

      HashSet<CodeObject::InsPtr, InsPtrHasher> ignored_blocks_;
      InsPtrHashTable<std::size_t> block_counts_;
      InsPtrHashTable<std::vector<CompilationResult>> compilation_results_;
      Compiler* compiler_;
    };
  }
}

#endif // LOXX_JIT_CODEPROFILER_HPP
