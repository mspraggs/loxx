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

#include "CodeProfiler.hpp"


namespace loxx
{
  void CodeProfiler::count_basic_block(
      const CodeObject* code, const CodeObject::InsPtr ip)
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

    if (hot_block_) {
      auto& block_info = *hot_block_;
      hot_block_ = nullptr;

      block_info.end = ip;

#ifndef NDEBUG
      if (debug_) {
        std::cout << "Compiling bytecode: "
                  << static_cast<const void*>(&(*block_info.begin)) << " -> "
                  << static_cast<const void*>(&(*block_info.end)) << '\n';
      }
#endif
    }
  }


  CodeProfiler::InstructionData::InstructionData(
      const Value* start, const std::size_t num_values)
      : data_on_stack_(num_values <= max_stack_values)
  {
    if (data_on_stack_) {
      std::transform(
          start, start + num_values, types_stack_.begin(),
          [] (const Value& value) {
            return static_cast<ValueType>(value.index());
          });
    }
    else {
      types_heap_.resize(num_values);
      std::transform(
          start, start + num_values, types_heap_.begin(),
          [] (const Value& value) {
            return static_cast<ValueType>(value.index());
          });
    }
  }


  std::size_t CodeProfiler::InsPtrHasher::operator() (
      const CodeObject::InsPtr ptr) const
  {
    return ptr_hasher(&(*ptr));
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
