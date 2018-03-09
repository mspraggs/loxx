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
 * Created by Matt Spraggs on 05/03/2018.
 */

#ifndef LOXX_VIRTUALMACHINE_HPP
#define LOXX_VIRTUALMACHINE_HPP

#include <vector>

#include "globals.hpp"
#include "Instruction.hpp"
#include "Variant.hpp"
#include "VirtualMachine.hpp"
#include "Stack.hpp"


namespace loxx
{
  class VirtualMachine
  {
  public:
    explicit VirtualMachine();

    void execute(const CompilationOutput& compiler_output);

    std::vector<StackVar>& constants() { return constants_; }

  private:
    void print_object(StackVar object) const;
    void execute_binary_op(const Instruction instruction);

    template <typename T>
    T read_integer();
    void check_number_operands(const StackVar& first,
                               const StackVar& second) const;

    std::size_t ip_;
    const CompilationOutput* compiler_output_;
    std::vector<StackVar> constants_;
    Stack<StackVar> stack_;
  };


  template <typename T>
  T VirtualMachine::read_integer()
  {
    T integer = 0;

    for (std::size_t i = 0; i < sizeof(T); ++i) {
      integer |= (compiler_output_->bytecode[++ip_] << 8 * i);
    }

    return integer;
  }
}

#endif // LOXX_VIRTUALMACHINE_HPP
