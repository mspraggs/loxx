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
 * Created by Matt Spraggs on 30/11/2019.
 */

#ifndef LOXX_JIT_REGISTERX86_HPP
#define LOXX_JIT_REGISTERX86_HPP

#include "../Value.hpp"

#include "Platform.hpp"


namespace loxx
{
  namespace jit
  {
    enum class RegisterX86 : std::uint8_t
    {
      RAX = 0,
      RCX = 1,
      RDX = 2,
      RBX = 3,
      RSP = 4,
      RBP = 5,
      RSI = 6,
      RDI = 7,
      R8 = 8,
      R9 = 9,
      R10 = 10,
      R11 = 11,
      R12 = 12,
      R13 = 13,
      R14 = 14,
      R15 = 15,
      XMM0 = 16,
      XMM1 = 17,
      XMM2 = 18,
      XMM3 = 19,
      XMM4 = 20,
      XMM5 = 21,
      XMM6 = 22,
      XMM7 = 23,
      XMM8 = 24,
      XMM9 = 25,
      XMM10 = 26,
      XMM11 = 27,
      XMM12 = 28,
      XMM13 = 29,
      XMM14 = 30,
      XMM15 = 31,
      RIP = 37,
    };


    template <>
    struct Reg<Platform::X86_64>
    {
      using Type = RegisterX86;
    };


    bool reg_is_64_bit(const RegisterX86 reg);


    bool reg_supports_float(const RegisterX86 reg);


    bool reg_supports_int(const RegisterX86 reg);


    bool reg_supports_ptr(const RegisterX86 reg);


    bool reg_supports_value_type(const RegisterX86 reg, const ValueType type);


    template <typename Os>
    Os& operator<<(Os& stream, RegisterX86 reg)
    {
      switch (reg) {
      case RegisterX86::RAX:
        stream << "rax";
        break;
      case RegisterX86::RBX:
        stream << "rbx";
        break;
      case RegisterX86::RCX:
        stream << "rcx";
        break;
      case RegisterX86::RDX:
        stream << "rdx";
        break;
      case RegisterX86::RSP:
        stream << "rsp";
        break;
      case RegisterX86::RBP:
        stream << "rbp";
        break;
      case RegisterX86::RSI:
        stream << "rsi";
        break;
      case RegisterX86::RDI:
        stream << "rdi";
        break;
      case RegisterX86::R8:
        stream << "r8";
        break;
      case RegisterX86::R9:
        stream << "r9";
        break;
      case RegisterX86::R10:
        stream << "r10";
        break;
      case RegisterX86::R11:
        stream << "r11";
        break;
      case RegisterX86::R12:
        stream << "r12";
        break;
      case RegisterX86::R13:
        stream << "r13";
        break;
      case RegisterX86::R14:
        stream << "r14";
        break;
      case RegisterX86::R15:
        stream << "r15";
        break;
      case RegisterX86::RIP:
        stream << "rip";
        break;
      case RegisterX86::XMM0:
        stream << "xmm0";
        break;
      case RegisterX86::XMM1:
        stream << "xmm1";
        break;
      case RegisterX86::XMM2:
        stream << "xmm2";
        break;
      case RegisterX86::XMM3:
        stream << "xmm3";
        break;
      case RegisterX86::XMM4:
        stream << "xmm4";
        break;
      case RegisterX86::XMM5:
        stream << "xmm5";
        break;
      case RegisterX86::XMM6:
        stream << "xmm6";
        break;
      case RegisterX86::XMM7:
        stream << "xmm7";
        break;
      case RegisterX86::XMM8:
        stream << "xmm8";
        break;
      case RegisterX86::XMM9:
        stream << "xmm9";
        break;
      case RegisterX86::XMM10:
        stream << "xmm10";
        break;
      case RegisterX86::XMM11:
        stream << "xmm11";
        break;
      case RegisterX86::XMM12:
        stream << "xmm12";
        break;
      case RegisterX86::XMM13:
        stream << "xmm13";
        break;
      case RegisterX86::XMM14:
        stream << "xmm14";
        break;
      case RegisterX86::XMM15:
        stream << "xmm15";
        break;
      }

      return stream;
    }
  }
}

#endif // LOXX_JIT_REGISTERX86_HPP
