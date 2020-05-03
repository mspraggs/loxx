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

#include <iostream>

#include "../CodeObject.hpp"

#include "Platform.hpp"
#include "PlatformX86.hpp"
#include "TraceCache.hpp"


extern "C" loxx::CodeObject::InsPtr asm_enter_x86_64(
    const loxx::jit::Trace* trace, const std::uint8_t* mcode,
    loxx::jit::ExitHandler exit_handler,
    loxx::Stack<loxx::Value, loxx::max_stack_size>* stack);

extern "C" void asm_exit_x86_64();

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
      if (reg == RegisterX86::RBP or reg == RegisterX86::RSP) {
        return true;
      }
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
    std::vector<RegisterX86> get_platform_registers<Platform::X86_64>()
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
    std::vector<RegisterX86> get_scratch_registers<Platform::X86_64>()
    {
      return {
        RegisterX86::R14,
        RegisterX86::R15,
        RegisterX86::XMM1,
      };
    }


    template <>
    auto get_exit_stub_pointer<Platform::X86_64>() -> void (*) ()
    {
      return &asm_exit_x86_64;
    }


    CodeObject::InsPtr handle_exit_x86_64(
        Trace* trace, const std::size_t exit_num,
        Stack<Value, max_stack_size>* stack)
    {
      return trace->snaps[exit_num].next_ip;
    }


    CodeObject::InsPtr LOXX_NOINLINE asm_enter_x86_64_impl(
        const Trace* trace,
        const std::uint8_t* mcode,
        loxx::jit::ExitHandler exit_handler,
        Stack<Value, max_stack_size>* stack)
    {
      asm volatile(
        ".globl asm_enter_x86_64\n"
        "asm_enter_x86_64:\n"
        "push %%rbp\n"
        "push %%rdi\n"   // trace
        "push %%rdx\n"   // exit_handler
        "push %%rcx\n"   // stack
        "jmpq *%%rsi\n"  // mcode
        : :
      );
    }


    void LOXX_NOINLINE asm_exit_x86_64_impl()
    {
      asm volatile(
        ".globl asm_exit_x86_64\n"
        "asm_exit_x86_64:\n"
        "pop %%rsi\n"    // exit_num
        "pop %%rdx\n"    // stack
        "pop %%r14\n"    // exit_handler
        "pop %%rdi\n"    // trace
        "callq *%%r14\n"
        "pop %%rbp\n"
        "ret\n"
        : :
      );
    }


    template <>
    CodeObject::InsPtr LOXX_NOINLINE execute_assembly<Platform::X86_64>(
        Trace* trace, Stack<Value, max_stack_size>* stack)
    {
      return asm_enter_x86_64(
          trace, trace->assembly.start(), &handle_exit_x86_64, stack);
    }
  }
}
