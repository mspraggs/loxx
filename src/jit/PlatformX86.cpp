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

#include "../CodeObject.hpp"

#include "Platform.hpp"
#include "PlatformX86.hpp"
#include "TraceCache.hpp"


extern "C" const std::uint8_t* asm_enter_x86_64(
    const loxx::jit::Trace* trace, const std::uint8_t* mcode,
    loxx::jit::ExitHandler<loxx::jit::Platform::X86_64> exit_handler,
    loxx::Stack<loxx::Value, loxx::max_stack_size>* stack);

extern "C" void asm_exit_x86_64();

namespace loxx
{
  namespace jit
  {
    template <>
    struct ExitState<Platform::X86_64>
    {
      std::array<double, 16> xmm_reg_vals;
      std::array<std::uint64_t, 16> gp_reg_vals;
    };


    bool reg_is_64_bit(const RegisterX86 reg)
    {
      return (static_cast<std::uint8_t>(reg) & 0x8) == 0x8;
    }


    bool reg_supports_float(const RegisterX86 reg)
    {
      return (static_cast<std::uint8_t>(reg) & 0x10) == 0x10;
    }


    bool reg_supports_int(const RegisterX86 reg)
    {
      return
          (static_cast<std::uint8_t>(reg) & 0x10) == 0 and
          reg != RegisterX86::RSP and
          reg != RegisterX86::RBP;
    }


    bool reg_supports_ptr(const RegisterX86 reg)
    {
      return (static_cast<std::uint8_t>(reg) & 0x10) == 0x00;
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


    std::size_t get_reg_index(const RegisterX86 reg)
    {
      const auto base = static_cast<std::size_t>(
          reg_supports_float(reg) ? RegisterX86::XMM0 : Register::RAX);
      return static_cast<std::size_t>(reg) - base;
    }


    template <>
    auto get_exit_stub_pointer<Platform::X86_64>() -> void (*) ()
    {
      return &asm_exit_x86_64;
    }


    CodeObject::InsPtr handle_exit_x86_64(
        Trace* trace, const std::size_t exit_num,
        Stack<Value, max_stack_size>* stack,
        const ExitState<Platform::X86_64>* exit_state)
    {
      const auto& snapshot = trace->snaps[exit_num];

      for (
          auto it = snapshot.stack_map_begin;
          it != snapshot.stack_map_end; ++it) {

        const auto slot = it->slot;
        const auto ir_ref = it->ir_ref;
        const auto& ir_instruction = trace->ir_buffer[ir_ref];
        const auto& allocation = trace->allocation_map[ir_ref];
        const auto reg = get<RegisterX86>(allocation.value());
        const auto reg_idx = get_reg_index(reg);

        const auto value = [&] {
          switch(ir_instruction.type()) {
          case ValueType::FLOAT:
            return Value(exit_state->xmm_reg_vals[reg_idx]);
          case ValueType::BOOLEAN:
            return Value(InPlace<bool>(), exit_state->gp_reg_vals[reg_idx]);
          case ValueType::OBJECT:
            return Value(
                InPlace<ObjectPtr>(),
                reinterpret_cast<ObjectPtr>(exit_state->gp_reg_vals[reg_idx]));
          case ValueType::UNKNOWN:
            return Value();
          }
        } ();

        if (stack->size() <= slot) {
          while (stack->size() < slot) {
            stack->emplace();
          }
          stack->push(value);
        }
        else {
          stack->set(slot, value);
        }
      }
      return trace->snaps[exit_num].next_ip;
    }


    CodeObject::InsPtr LOXX_NOINLINE asm_enter_x86_64_impl(
        const Trace*, const std::uint8_t*, ExitHandler<Platform::X86_64>,
        Stack<Value, max_stack_size>*)
    {
      asm volatile(
        ".globl asm_enter_x86_64\n"
        "asm_enter_x86_64:\n"
        "push %%rbp\n"
        "push %%r15\n"
        "push %%r14\n"
        "push %%r13\n"
        "push %%r12\n"
        "push %%rbx\n"
        "movq %%rsp, %%rbp\n"
        "push %%rdi\n"   // trace
        "push %%rdx\n"   // exit_handler
        "push %%rcx\n"   // stack
        "jmpq *%%rsi\n"  // mcode
        : :
      );

      return {};
    }


    void LOXX_NOINLINE asm_exit_x86_64_impl()
    {
      asm volatile(
        ".globl asm_exit_x86_64\n"
        "asm_exit_x86_64:\n"
        // Saver registers on the stack
        "push %%r15\n"
        "push %%r14\n"
        "push %%r13\n"
        "push %%r12\n"
        "push %%r11\n"
        "push %%r10\n"
        "push %%r9\n"
        "push %%r8\n"
        "push %%rdi\n"
        "push %%rsi\n"
        "push %%rbp\n"
        "push %%rsp\n"
        "push %%rdx\n"
        "push %%rcx\n"
        "push %%rbx\n"
        "push %%rax\n"
        "subq $128, %%rsp\n"
        "movsd %%xmm0,  -288(%%rbp)\n"
        "movsd %%xmm1,  -280(%%rbp)\n"
        "movsd %%xmm2,  -272(%%rbp)\n"
        "movsd %%xmm3,  -264(%%rbp)\n"
        "movsd %%xmm4,  -256(%%rbp)\n"
        "movsd %%xmm5,  -248(%%rbp)\n"
        "movsd %%xmm6,  -240(%%rbp)\n"
        "movsd %%xmm7,  -232(%%rbp)\n"
        "movsd %%xmm8,  -224(%%rbp)\n"
        "movsd %%xmm9,  -216(%%rbp)\n"
        "movsd %%xmm10, -208(%%rbp)\n"
        "movsd %%xmm11, -200(%%rbp)\n"
        "movsd %%xmm12, -192(%%rbp)\n"
        "movsd %%xmm13, -184(%%rbp)\n"
        "movsd %%xmm14, -176(%%rbp)\n"
        "movsd %%xmm15, -168(%%rbp)\n"
        // Set up call to exit handler
        "mov -32(%%rbp),  %%rsi\n"    // exit_num
        "mov -24(%%rbp),  %%rdx\n"    // stack
        "mov -16(%%rbp),  %%r14\n"    // exit_handler
        "mov -8(%%rbp),   %%rdi\n"    // trace
        "lea -288(%%rbp), %%rcx\n"    // constructed exit_state
        "callq *%%r14\n"
        // Fix up stack pointer - we've use 32 + 128 + 128 = 288 bytes
        "addq $288, %%rsp\n"
        "pop %%rbx\n"
        "pop %%r12\n"
        "pop %%r13\n"
        "pop %%r14\n"
        "pop %%r15\n"
        "pop %%rbp\n"
        "ret\n"
        : :
      );
    }


    template <>
    CodeObject::InsPtr LOXX_NOINLINE execute_assembly<Platform::X86_64>(
        Trace* trace, Stack<Value, max_stack_size>* stack)
    {
      const auto& bytecode = trace->code_object->bytecode;
      const auto ip_ptr = asm_enter_x86_64(
          trace, trace->assembly.start(), &handle_exit_x86_64, stack);
      return bytecode.begin() + std::distance(bytecode.data(), ip_ptr);
    }
  }
}
