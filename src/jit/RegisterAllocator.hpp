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
 * Created by Matt Spraggs on 23/10/2019.
 */

#ifndef LOXX_JIT_REGISTERALLOCATOR_HPP
#define LOXX_JIT_REGISTERALLOCATOR_HPP

#include <utility>

#include "SSAGenerator.hpp"
#include "SSAInstruction.hpp"


namespace loxx
{
  namespace jit
  {
    using Range = std::pair<std::size_t, std::size_t>;


    std::vector<std::pair<Operand, Range>> compute_live_ranges(
        const std::vector<SSAInstruction<2>>& ssa_ir);


    class RegisterAllocator
    {
    public:
      explicit RegisterAllocator(const bool debug) : debug_(debug) {}

      void allocate(const std::vector<SSAInstruction<2>>& ssa_ir);

    private:
      bool debug_;
    };
  }
}

#endif // LOXX_JIT_REGISTERALLOCATOR_HPP
