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
    enum class RegisterX86
    {
      RAX,
      RBX,
      RCX,
      RDX,
      RSP,
      RBP,
      RSI,
      RDI,
      R8,
      R9,
      R10,
      R11,
      R12,
      R13,
      R14,
      R15,
      RIP,
      XMM0,
      XMM1,
      XMM2,
      XMM3,
      XMM4,
      XMM5,
      XMM6,
      XMM7,
      XMM8,
      XMM9,
      XMM10,
      XMM11,
      XMM12,
      XMM13,
      XMM14,
      XMM15,
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
