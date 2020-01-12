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

#include <numeric>

#include "AssemblerX86.hpp"
#include "JITError.hpp"


namespace loxx
{
  namespace jit
  {
    std::uint8_t get_reg_rm_bits(const RegisterX86 reg)
    {
      switch (reg) {
      case RegisterX86::RAX:
      case RegisterX86::R8:
      case RegisterX86::XMM0:
      case RegisterX86::XMM8:
        return 0b000;
      case RegisterX86::RCX:
      case RegisterX86::R9:
      case RegisterX86::XMM1:
      case RegisterX86::XMM9:
        return 0b001;
      case RegisterX86::RDX:
      case RegisterX86::R10:
      case RegisterX86::XMM2:
      case RegisterX86::XMM10:
        return 0b010;
      case RegisterX86::RBX:
      case RegisterX86::R11:
      case RegisterX86::XMM3:
      case RegisterX86::XMM11:
        return 0b011;
      case RegisterX86::RSP:
      case RegisterX86::R12:
      case RegisterX86::XMM4:
      case RegisterX86::XMM12:
        return 0b100;
      case RegisterX86::RBP:
      case RegisterX86::R13:
      case RegisterX86::XMM5:
      case RegisterX86::XMM13:
        return 0b101;
      case RegisterX86::RSI:
      case RegisterX86::R14:
      case RegisterX86::XMM6:
      case RegisterX86::XMM14:
        return 0b110;
      case RegisterX86::RDI:
      case RegisterX86::R15:
      case RegisterX86::XMM7:
      case RegisterX86::XMM15:
        return 0b111;
      default:
        return 0b000;
      }
    }


    std::uint8_t get_rex_prefix_for_regs(
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      std::uint8_t ret = 0;
      std::array<std::pair<int, RegisterX86>, 2> shift_reg_mapping = {
        std::make_pair(0, reg0), std::make_pair(2, reg1)
      };

      for (const auto& mapping : shift_reg_mapping) {
        if (reg_is_64_bit(mapping.second)) {
          ret |= (1 << mapping.first);
        }
      }

      return ret;
    }


    AssemblyFunction Assembler<RegisterX86>::assemble(
        const std::vector<SSAInstruction<2>>& ssa_ir,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      add_push(RegisterX86::RBP);
      add_move_reg_reg(RegisterX86::RBP, RegisterX86::RSP);
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


    void Assembler<RegisterX86>::add_move_reg_reg(
        const RegisterX86 dst, const RegisterX86 src)
    {
      if (reg_supports_ptr(src) and reg_supports_ptr(dst)) {
        const std::uint8_t rex_prefix =
            0b01001000 | get_rex_prefix_for_regs(dst, src);
        const std::uint8_t mod_rm_byte =
            0b11000000 | (get_reg_rm_bits(src) << 3) | get_reg_rm_bits(dst);
        func_.add_byte(rex_prefix);
        func_.add_byte(0x89);
        func_.add_byte(mod_rm_byte);
      }
      else if (reg_supports_float(src) and reg_supports_float(dst)) {

      }
      else {
        throw JITError("invalid move registers");
      }
    }


    void Assembler<RegisterX86>::add_move_reg_mem(
        const RegisterX86 dst, const RegisterX86 src, const unsigned int offset)
    {
      if (reg_supports_ptr(dst)) {
        add_move_reg_to_from_mem(dst, src, offset, true);
      }
      else if (reg_supports_float(dst)) {

      }
      else {
        throw JITError("invalid move registers");
      }
    }



    void Assembler<RegisterX86>::add_move_reg_imm(
        const RegisterX86 dst, const std::uint64_t value)
    {
      const std::uint8_t rex_prefix =
          reg_is_64_bit(dst) ? 0b01001001 : 0b01001000;
      func_.add_byte(rex_prefix);
      func_.add_byte(0xb8 | get_reg_rm_bits(dst));

      const auto next_byte = [&] {
        static int i = 0;
        return static_cast<std::uint8_t>(0xff & (value >> (8 * (i++))));
      };
      std::array<std::uint8_t, 8> bytes;
      std::generate(bytes.begin(), bytes.end(), next_byte);
      func_.add_bytes(bytes.begin(), bytes.end());
    }


    void Assembler<RegisterX86>::add_move_reg_to_from_mem(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int offset, const bool read)
    {
      const std::uint8_t offset_bits = [&] {
        if (offset == 0) {
          return 0b00000000;
        }
        if (offset < 256) {
          return 0b01000000;
        }
        return 0b10000000;
      } ();

      const std::uint8_t rex_prefix =
          0b01001000 | get_rex_prefix_for_regs(src, dst);
      const std::uint8_t mod_rm_byte =
          offset_bits | (get_reg_rm_bits(dst  ) << 3) | get_reg_rm_bits(src);
      func_.add_byte(rex_prefix);
      func_.add_byte(read ? 0x8b : 0x89);
      func_.add_byte(mod_rm_byte);

      if (offset >= 256) {
        const auto next_byte = [&] {
          static int i = 0;
          return static_cast<std::uint8_t>(0xff & (offset >> (8 * (i++))));
        };
        std::array<std::uint8_t, 4> bytes;
        std::generate(bytes.begin(), bytes.end(), next_byte);
        func_.add_bytes(bytes.begin(), bytes.end());
      }
      else if (offset > 0) {
        func_.add_byte(static_cast<std::uint8_t>(0xff & offset));
      }}
  }
}
