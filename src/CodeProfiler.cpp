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

    if (count_elem->second >= block_count_threshold_) {
      hot_block_start_ = ip;
    }
  }


  void CodeProfiler::profile_instruction(
      const CodeObject::InsPtr ip,
      const Value* start, const std::size_t size)
  {
    if (not hot_block_start_) {
      return;
    }

    instruction_data_[ip] = InstructionData(start, size);
  }


  void CodeProfiler::flag_block_boundary(const CodeObject::InsPtr ip)
  {
    block_boundary_flagged_ = true;

    if (hot_block_start_) {
      const auto block_start = *hot_block_start_;
      hot_block_start_.reset();

      if (debug_) {
        std::cout << "Compiling block @ "
                  << static_cast<const void*>(&(*block_start)) << '\n';
      }
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
    return ip_hasher(info.ip);
  }


  bool CodeProfiler::BlockInfoCompare::operator() (
      const BlockInfo& info1, const BlockInfo& info2) const
  {
    return info1.ip == info2.ip;
  }
}
