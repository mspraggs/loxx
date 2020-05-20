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
 * Created by Matt Spraggs on 23/11/2019.
 */

#ifndef LOXX_JIT_REGISTERALLOCATOR_HPP
#define LOXX_JIT_REGISTERALLOCATOR_HPP

#include <utility>

#include "../HashSet.hpp"

#include "IRInstruction.hpp"
#include "Register.hpp"
#include "Snapshot.hpp"


namespace loxx
{
  namespace jit
  {
    struct Trace;


    using Range = std::pair<std::size_t, std::size_t>;


    std::vector<std::pair<std::size_t, Range>> compute_live_ranges(
        const std::vector<IRInstruction<2>>& ir_buffer,
        const std::vector<Snapshot>& snapshots);


    template <typename Reg>
    using Allocation = Variant<Reg, std::size_t>;


    template <typename Reg>
    using AllocationMap = std::vector<Optional<Allocation<Reg>>>;


    class RegisterAllocator
    {
    public:
      RegisterAllocator(
          const bool debug, const std::vector<Register>& registers);

      void allocate(Trace& trace);

    private:
      void expire_old_intervals(const Range& interval);
      void spill_at_interval(const Range& interval);
      void insert_active_interval(const Range& interval);
      void remove_active_interval(const Range& interval);
      Optional<Register> get_register(const ValueType type);
      void add_register(const Register reg);

      bool debug_;
      std::size_t stack_index_;
      // HashTable<std::size_t, Register> registers_;
      // HashTable<std::size_t, std::size_t> stack_slots_;
      std::vector<Optional<Register>> registers_;
      std::vector<Optional<std::size_t>> stack_slots_;
      std::vector<bool> register_pool_;
      std::vector<Range> active_intervals_;
      Trace* trace_;
    };
  }
}

#endif // LOXX_JIT_REGISTERALLOCATOR_HPP
