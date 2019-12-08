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
#include "../VirtualMachine.hpp"

#include "CodeProfiler.hpp"
#include "JITError.hpp"
#include "logging.hpp"


namespace loxx
{
  namespace jit
  {
    void CodeProfiler::count_basic_block(
        const CodeObject::InsPtr ip,
        const RuntimeContext& context)
    {
      block_boundary_flagged_ = false;
      const auto block_info = BlockInfo{&context.code, ip};
      auto count_elem = block_counts_.insert(block_info, 0);
      count_elem.first->second += 1;

  #ifndef NDEBUG
      if (debug_) {
        std::cout << "Block hit count @ " << static_cast<const void*>(&(*ip))
                  << " = " << count_elem.first->second << '\n';
      }
  #endif

      if (count_elem.first->second >= block_count_threshold_) {
        hot_block_ = &count_elem.first->first;
        compiler_->build_context(context);
      }
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

      try {
        compiler_->compile(block_info.begin, block_info.end);
      }
      catch (const JITError& err) {}
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
