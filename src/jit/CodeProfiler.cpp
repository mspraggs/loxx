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

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>

#include "../logging.hpp"

#include "CodeProfiler.hpp"
#include "logging.hpp"


namespace loxx
{
  namespace jit
  {
    InstructionData::InstructionData(
        const Value* start, const std::size_t num_values)
        : data_on_stack_(num_values <= max_stack_values)
    {
      if (data_on_stack_) {
        std::transform(
            start, start + num_values, types_stack_.begin(),
            [] (const Value& value) { return &value; });
      }
      else {
        types_heap_.resize(num_values);
        std::transform(
            start, start + num_values, types_heap_.begin(),
            [] (const Value& value) { return &value; });
      }
    }


    const Value& InstructionData::operator[] (const std::size_t i) const
    {
      if (data_on_stack_) {
        return *types_stack_[i];
      }
      return *types_heap_[i];
    }


    void CodeProfiler::count_basic_block(
        const CodeObject* code, const CodeObject::InsPtr ip,
        const Stack<Value, max_stack_size>& stack)
    {
      block_boundary_flagged_ = false;
      const auto block_info = BlockInfo{code, ip};
      auto& count_elem = block_counts_.insert(block_info, 0);
      count_elem->second += 1;

  #ifndef NDEBUG
      if (debug_) {
        std::cout << "Block hit count @ " << static_cast<const void*>(&(*ip))
                  << " = " << count_elem->second << '\n';
      }
  #endif

      if (count_elem->second >= block_count_threshold_) {
        hot_block_ = &count_elem->first;
        ssa_generator_.build_stack(stack);
      }
    }


    void CodeProfiler::profile_instruction(
        const CodeObject::InsPtr ip,
        const Value* start, const std::size_t size)
    {
      if (not hot_block_) {
        return;
      }

      instruction_data_[ip] = InstructionData(start, size);
    }


    void CodeProfiler::flag_block_boundary(const CodeObject::InsPtr ip)
    {
      block_boundary_flagged_ = true;

      if (not hot_block_) {
        return;
      }

      auto& block_info = *hot_block_;
      hot_block_ = nullptr;

      block_info.end = ip;

  #ifndef NDEBUG
      if (debug_) {
        std::cout << "=== Compiling Bytecode ===\n";
        print_bytecode(*block_info.code, block_info.begin, block_info.end);
      }
  #endif
    }


    std::size_t CodeProfiler::BlockInfoHasher::operator() (
        const BlockInfo& info) const
    {
      return ip_hasher(info.begin);
    }


    bool CodeProfiler::BlockInfoCompare::operator() (
        const BlockInfo& info1, const BlockInfo& info2) const
    {
      return info1.begin == info2.begin;
    }
  }
}
