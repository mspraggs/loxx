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

#ifndef LOXX_CODEPROFILER_HPP
#define LOXX_CODEPROFILER_HPP

#include <tuple>
#include <vector>

#include "CodeObject.hpp"
#include "Object.hpp"
#include "Value.hpp"


namespace loxx
{
  class CodeProfiler
  {
  public:
    CodeProfiler(const bool debug, const std::size_t block_count_threshold)
        : debug_(debug), block_boundary_flagged_(true),
          block_count_threshold_(block_count_threshold), hot_block_(nullptr)
    {
    }

    void count_basic_block(
        const CodeObject* code, const CodeObject::InsPtr ip);

    template <typename... Ts>
    void profile_instruction(
        const CodeObject::InsPtr ip, const Value& op0, const Ts&... ops);

    void profile_instruction(
        const CodeObject::InsPtr ip,
        const Value* start, const std::size_t size);

    void flag_block_boundary(const CodeObject::InsPtr ip);

    bool is_profiling_instructions() const { return hot_block_; }
    bool block_boundary_flagged() const { return block_boundary_flagged_; }

  private:
    struct TypeInfo
    {
      const CodeObject* code;
      const StringObject* varname;
      ValueType type;
    };

    using CallSignature = std::vector<ValueType>;

    struct CallInfo
    {
      const CodeObject* code;
      const ClosureObject* function;
      CallSignature signature;
    };

    struct BlockInfo
    {
      const CodeObject* code;
      CodeObject::InsPtr begin;
      CodeObject::InsPtr end;
    };

    class InstructionData
    {
    public:
      InstructionData() = default;
      template <typename... Ts>
      InstructionData(const Value& value0, const Ts&... values);
      InstructionData(const Value* start, const std::size_t num_values);

    private:
      static constexpr std::size_t max_stack_values = 3;
      bool data_on_stack_;
      std::array<ValueType, max_stack_values> types_stack_;
      std::vector<ValueType> types_heap_;
    };

    struct InsPtrHasher
    {
      std::size_t operator() (const CodeObject::InsPtr ptr) const;

      std::hash<const std::uint8_t*> ptr_hasher;
    };

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
    HashTable<CodeObject::InsPtr, InstructionData, InsPtrHasher>
        instruction_data_;
  };


  template <typename... Ts>
  void CodeProfiler::profile_instruction(
      const CodeObject::InsPtr ip, const Value& op0, const Ts&... ops)
  {
    if (not hot_block_) {
      return;
    }

    instruction_data_[ip] = InstructionData(op0, ops...);
  }


  template <typename... Ts>
  CodeProfiler::InstructionData::InstructionData(
      const Value& value0, const Ts&... values)
      : data_on_stack_(sizeof...(Ts) <= max_stack_values)
  {
    if (data_on_stack_) {
      types_stack_ = {
        static_cast<ValueType>(value0.index()),
        static_cast<ValueType>(values.index())...
      };
    }
    else {
      types_heap_ = {
        static_cast<ValueType>(value0.index()),
        static_cast<ValueType>(values.index())...
      };
    }
  }
}

#endif // LOXX_CODEPROFILER_HPP
