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


    std::vector<Range> compute_live_ranges(
        const std::vector<IRInstruction<2>>& ir_buffer,
        const std::vector<Snapshot>& snapshots);


    template <typename Reg>
    using Allocation = Variant<Reg, std::size_t>;


    template <typename Reg>
    using AllocationMap = std::vector<Optional<Allocation<Reg>>>;


    class RegisterAllocator
    {
    public:
      RegisterAllocator(const std::vector<Register>& registers);

      void allocate(Trace& trace);

    private:
      void expire_old_intervals(const Range& interval);
      void spill_at_interval(const Range& interval);
      void insert_active_interval(const Range& interval);
      void remove_active_interval(const Range& interval);
      Optional<Register> get_register(const ValueType type);
      void add_register(const Register reg);

      std::size_t stack_index_;
      std::array<bool, 128> register_pool_;
    };
  }
}

#endif // LOXX_JIT_REGISTERALLOCATOR_HPP
