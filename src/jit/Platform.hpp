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

#ifndef LOXX_JIT_PLATFORM_HPP
#define LOXX_JIT_PLATFORM_HPP

#include <algorithm>
#include <vector>

#include "../CodeObject.hpp"
#include "../globals.hpp"
#include "../Stack.hpp"

#include "AssemblyWrapper.hpp"


namespace loxx
{
  namespace jit
  {
    struct Trace;


    enum class Platform
    {
      X86_64,
    };


    using ExitHandler = CodeObject::InsPtr (*) (
        Trace*, const std::size_t, Stack<Value, max_stack_size>*);


    template <Platform P>
    struct Reg;


    template <Platform P>
    using RegType = typename Reg<P>::Type;


    template <Platform P>
    std::vector<RegType<P>> get_platform_registers();


    template <Platform P>
    std::vector<RegType<P>> get_scratch_registers();


    template <Platform P>
    std::vector<RegType<P>> get_allocatable_registers()
    {
      const auto scratch_registers = get_scratch_registers<P>();
      auto available_registers = get_platform_registers<P>();

      for (const auto scratch_register : scratch_registers) {
        const auto pos = std::find(
            available_registers.begin(), available_registers.end(),
            scratch_register);
        available_registers.erase(pos);
      }

      return available_registers;
    }


    template <Platform P>
    auto get_exit_stub_pointer() -> void (*) ();


    template <Platform P>
    CodeObject::InsPtr LOXX_NOINLINE execute_assembly(
        Trace* trace, Stack<Value, max_stack_size>* stack);


    constexpr Platform platform = Platform::X86_64;
  }
}

#endif // LOXX_JIT_PLATFORM_HPP
