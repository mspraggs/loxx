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


    bool reg_supports_value_type(const RegisterX86 reg, const ValueType type);
  }
}

#endif // LOXX_JIT_REGISTERX86_HPP
