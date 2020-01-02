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
 * Created by Matt Spraggs on 22/11/2019.
 */

#include "AssemblerX86.hpp"


namespace loxx
{
  namespace jit
  {
    std::uint8_t get_reg_rm_bits(const RegisterX86 reg)
    {
      switch (reg) {
      case RegisterX86::RAX:
      case RegisterX86::XMM0:
        return 0b000;
      case RegisterX86::RCX:
      case RegisterX86::XMM1:
        return 0b001;
      case RegisterX86::RDX:
      case RegisterX86::XMM2:
        return 0b010;
      case RegisterX86::RBX:
      case RegisterX86::XMM3:
        return 0b011;
      case RegisterX86::RSP:
      case RegisterX86::XMM4:
        return 0b100;
      case RegisterX86::RBP:
      case RegisterX86::XMM5:
        return 0b101;
      case RegisterX86::RSI:
      case RegisterX86::XMM6:
        return 0b110;
      case RegisterX86::RDI:
      case RegisterX86::XMM7:
        return 0b111;
      default:
        return 0b000;
      }
    }


    AssemblyFunction Assembler<RegisterX86>::assemble(
        const std::vector<SSAInstruction<2>>& ssa_ir,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      add_push(RegisterX86::RBP);
      add_pop(RegisterX86::RBP);
      add_return();
      return func_;
    }


    void Assembler<RegisterX86>::add_return()
    {
      func_.add_byte(0xc3);
    }


    void Assembler<RegisterX86>::add_push(const RegisterX86 src)
    {
      func_.add_byte(0x50 | get_reg_rm_bits(src));
    }


    void Assembler<RegisterX86>::add_pop(const RegisterX86 dst)
    {
      func_.add_byte(0x58 | get_reg_rm_bits(dst));
    }
  }
}
