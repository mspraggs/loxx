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
 * Created by Matt Spraggs on 08/12/2019.
 */

#include "Register.hpp"
#include "RegisterX86.hpp"


namespace loxx
{
  namespace jit
  {
    bool reg_is_64_bit(const RegisterX86 reg)
    {
      switch (reg) {
        case RegisterX86::R8:
        case RegisterX86::R9:
        case RegisterX86::R10:
        case RegisterX86::R11:
        case RegisterX86::R12:
        case RegisterX86::R13:
        case RegisterX86::R14:
        case RegisterX86::R15:
        case RegisterX86::XMM8:
        case RegisterX86::XMM9:
        case RegisterX86::XMM10:
        case RegisterX86::XMM11:
        case RegisterX86::XMM12:
        case RegisterX86::XMM13:
        case RegisterX86::XMM14:
        case RegisterX86::XMM15:
          return true;
        default:
          return false;
        }
    }


    bool reg_supports_float(const RegisterX86 reg)
    {
      switch (reg) {
      case RegisterX86::XMM0:
      case RegisterX86::XMM1:
      case RegisterX86::XMM2:
      case RegisterX86::XMM3:
      case RegisterX86::XMM4:
      case RegisterX86::XMM5:
      case RegisterX86::XMM6:
      case RegisterX86::XMM7:
      case RegisterX86::XMM8:
      case RegisterX86::XMM9:
      case RegisterX86::XMM10:
      case RegisterX86::XMM11:
      case RegisterX86::XMM12:
      case RegisterX86::XMM13:
      case RegisterX86::XMM14:
      case RegisterX86::XMM15:
        return true;
      default:
        return false;
      }
    }


    bool reg_supports_int(const RegisterX86 reg)
    {
      switch (reg) {
      case RegisterX86::RAX:
      case RegisterX86::RBX:
      case RegisterX86::RCX:
      case RegisterX86::RDX:
      case RegisterX86::RSI:
      case RegisterX86::RDI:
      case RegisterX86::R8:
      case RegisterX86::R9:
      case RegisterX86::R10:
      case RegisterX86::R11:
      case RegisterX86::R12:
      case RegisterX86::R13:
      case RegisterX86::R14:
      case RegisterX86::R15:
        return true;
      default:
        return false;
      }
    }


    bool reg_supports_ptr(const RegisterX86 reg)
    {
      return reg_supports_int(reg);
    }


    bool reg_supports_value_type(const RegisterX86 reg, const ValueType type)
    {
      if (type == ValueType::FLOAT) {
        return reg_supports_float(reg);
      }
      return reg_supports_ptr(reg);
    }


    template <>
    std::vector<RegisterX86> get_platform_registers<RegisterX86>()
    {
      return {
        RegisterX86::RAX,
        RegisterX86::RBX,
        RegisterX86::RCX,
        RegisterX86::RDX,
        RegisterX86::RSI,
        RegisterX86::RDI,
        RegisterX86::R8,
        RegisterX86::R9,
        RegisterX86::R10,
        RegisterX86::R11,
        RegisterX86::R12,
        RegisterX86::R13,
        RegisterX86::R14,
        RegisterX86::R15,
        RegisterX86::XMM0,
        RegisterX86::XMM1,
        RegisterX86::XMM2,
        RegisterX86::XMM3,
        RegisterX86::XMM4,
        RegisterX86::XMM5,
        RegisterX86::XMM6,
        RegisterX86::XMM7,
        RegisterX86::XMM8,
        RegisterX86::XMM9,
        RegisterX86::XMM10,
        RegisterX86::XMM11,
        RegisterX86::XMM12,
        RegisterX86::XMM13,
        RegisterX86::XMM14,
        RegisterX86::XMM15,
      };
    }


    template <>
    std::vector<RegisterX86> get_scratch_registers<RegisterX86>()
    {
      return {
        RegisterX86::RDX,
        RegisterX86::XMM1,
      };
    }
  }
}
