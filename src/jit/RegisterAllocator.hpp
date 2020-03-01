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

#include "PlatformX86.hpp"
#include "SSAGenerator.hpp"
#include "SSAInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    using Range = std::pair<std::size_t, std::size_t>;


    std::vector<std::pair<VirtualRegister, Range>> compute_live_ranges(
        const std::vector<SSAInstruction<2>>& ssa_ir);


    template <typename Reg>
    using Allocation = Variant<Reg, std::size_t>;


    template <typename Reg>
    using AllocationMap =
        HashTable<VirtualRegister, Allocation<Reg>,
        VirtualRegisterHasher, VirtualRegisterCompare>;


    class RegisterAllocator
    {
    public:
      RegisterAllocator(
          const bool debug, const std::vector<RegType<platform>>& registers);

      AllocationMap<RegType<platform>> allocate(const SSABuffer<3>& ssa_ir);

    private:
      void expire_old_intervals(const Range& interval);
      void spill_at_interval(const Range& interval);
      void insert_active_interval(const Range& interval);
      void remove_active_interval(const Range& interval);
      Optional<RegType<platform>> get_register(const ValueType type);

      bool debug_;
      std::size_t stack_index_;
      HashTable<std::size_t, RegType<platform>> registers_;
      HashTable<std::size_t, std::size_t> stack_slots_;
      HashSet<RegType<platform>> register_pool_;
      std::vector<Range> active_intervals_;
    };
  }
}

#endif // LOXX_JIT_REGISTERALLOCATOR_HPP
