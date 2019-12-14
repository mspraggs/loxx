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
 * Created by Matt Spraggs on 09/12/2019.
 */

#ifndef LOXX_JIT_REGISTER_HPP
#define LOXX_JIT_REGISTER_HPP

#include <algorithm>
#include <vector>

#include "RegisterX86.hpp"


namespace loxx
{
  namespace jit
  {
    template <typename Reg>
    std::vector<Reg> get_platform_registers();


    template <typename Reg>
    std::vector<Reg> get_scratch_registers();


    template <typename Reg>
    std::vector<Reg> get_allocatable_registers()
    {
      const auto scratch_registers = get_scratch_registers<Reg>();
      auto available_registers = get_platform_registers<Reg>();

      for (const auto scratch_register : scratch_registers) {
        const auto pos =  std::find(
            available_registers.begin(), available_registers.end(),
            scratch_register);
        available_registers.erase(pos);
      }

      return available_registers;
    }


    using Register = RegisterX86;
  }
}

#endif // LOXX_JIT_REGISTER_HPP
