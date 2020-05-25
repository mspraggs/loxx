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
 * Created by Matt Spraggs on 15/11/2019.
 */

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "logging.hpp"
#include "IRInstruction.hpp"

namespace loxx
{
  namespace jit
  {
    void print_trace(const Trace& trace)
    {
      const auto& snaps = trace.snaps;
      std::size_t snap_counter = 0;

      for (std::size_t i = 0; i < trace.ir_buffer.size(); ++i) {
        while (snaps[snap_counter].ir_ref == i and
            snap_counter < snaps.size()) {
          print_snapshot(snap_counter, snaps[snap_counter]);
          ++snap_counter;
        }
        std::cout << std::setw(4) << std::setfill('0') << std::right
                  << i << "  " << std::left;
        const auto& allocation = trace.allocation_map[i];
        if (allocation) {
          if (holds_alternative<Register>(*allocation)) {
            const auto reg = unsafe_get<Register>(*allocation);
            std::cout << std::setw(6) << std::setfill(' ') << reg << ' ';
          }
          else {
            const auto slot = unsafe_get<std::size_t>(*allocation);
            std::cout << '+' << std::setw(5) << std::setfill(' ') << slot << ' ';
          }
        }
        else {
          std::cout << "       ";
        }
        print_ssa_instruction(trace, trace.ir_buffer[i]);
      }
    }


    void print_live_ranges(
        const std::vector<std::pair<std::size_t, Range>>& live_ranges)
    {
      using RegisterRangePair = std::pair<std::size_t, Range>;

      const auto range_max_elem = std::max_element(
          live_ranges.begin(), live_ranges.end(),
          [] (const RegisterRangePair& first, const RegisterRangePair& second) {
            return first.second.second < second.second.second;
          }
      );
      const auto range_max = range_max_elem->second.second;

      std::cout << "      ";
      for (std::size_t i = 0; i < range_max + 1; ++i) {
        std::cout << i / 10;
      }
      std::cout << '\n';

      std::cout << "      ";
      for (std::size_t i = 0; i < range_max + 1; ++i) {
        std::cout << i % 10;
      }
      std::cout << '\n';

      for (const auto& live_range : live_ranges) {
        const auto begin = live_range.second.first;
        const auto end = live_range.second.second;

        std::stringstream ss;
        ss << live_range.first;

        std::cout << std::left << std::setw(6) << ss.str();
        for (std::size_t i = 0; i < end + 1; ++i) {
          if (i < live_range.second.first) {
            std::cout << ' ';
          }
          else if (i == end) {
            std::cout << (begin < end ? '>' : 'v');
          }
          else {
            std::cout << '-';
          }
        }
        std::cout << '\n';
      }
    }


    void print_allocation_map(
        const AllocationMap<Register>& allocation_map)
    {
      std::cout << "=== Register Allocation Map ===\n";
      for (unsigned int ref = 0; ref < allocation_map.size(); ++ref) {
        const auto& allocation = allocation_map[ref];
        if (not allocation) {
          continue;
        }
        const auto is_register = holds_alternative<Register>(*allocation);
        std::cout << ref << " -> ";
        if (is_register) {
          std::cout << unsafe_get<Register>(*allocation) << "\n";
        }
        else {
          std::cout << "[ +" << std::right << std::setw(3) << std::setfill('0')
                    << unsafe_get<std::size_t>(*allocation) << " ]\n";
        }
      }
    }


    void print_snapshot(const std::size_t snap_num, const Snapshot& snapshot)
    {
      std::size_t stack_pos = 0;
      std::cout << "....         SNAPSHOT       #" << std::setw(3)
                << std::setfill(' ') << snap_num << "      ";

      for (const auto& mapping : snapshot.stack_ir_map) {
        while (stack_pos < mapping.first) {
          std::cout << "[ ---- ] ";
          ++stack_pos;
        }

        std::cout << "[ " << std::setw(4) << std::setfill('0') << std::right
                  << mapping.second.value << " ] ";
        ++stack_pos;
      }

      std::cout << std::left << '\n';
    }
  }
}
