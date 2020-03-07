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
 * Created by Matt Spraggs on 21/12/2018.
 */

#ifndef LOXX_JIT_ASSEMBLERX86_HPP
#define LOXX_JIT_ASSEMBLERX86_HPP

#include "Assembler.hpp"


namespace loxx
{
  namespace jit
  {
    enum class Condition {
      ABOVE,
      ABOVE_OR_EQUAL,
      BELOW,
      BELOW_OR_EQUAL,
      EQUAL,
      NOT_EQUAL,
      GREATER,
      GREATER_OR_EQUAL,
      LESS,
      LESS_OR_EQUAL,
    };


    template <>
    class Assembler<Platform::X86_64>
    {
    public:
      using SSAInstruction = SSAInstruction<3>;

      Assembler();

      AssemblyWrapper assemble(
          const std::vector<SSAInstruction>& ssa_ir,
          const AllocationMap<RegisterX86>& allocation_map);

    private:
      void insert_type_guards(const ReferenceSet& operands);

      void emit_guard(const Value* location, const ValueType type);

      void emit_add(
          const SSAInstruction& instruction,
          const AllocationMap<RegisterX86>& allocation_map);
      void emit_move(
          const SSAInstruction& instruction,
          const AllocationMap<RegisterX86>& allocation_map);
      void emit_multiply(
          const SSAInstruction& instruction,
          const AllocationMap<RegisterX86>& allocation_map);

      void emit_return();
      void emit_push(const RegisterX86 src);
      void emit_pop(const RegisterX86 dst);

      void emit_add_reg_reg(
          const RegisterX86 reg0, const RegisterX86 reg1);
      void emit_multiply_reg_reg(
          const RegisterX86 reg0, const RegisterX86 reg1);

      void emit_decrement(const RegisterX86 reg);

      void emit_move_reg_reg(const RegisterX86 dst, const RegisterX86 src);
      void emit_move_reg_mem(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int displacement = 0);
      void emit_move_mem_reg(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int displacement = 0);
      void emit_move_reg_imm(
          const RegisterX86 dst, const std::uint64_t value);

      void emit_compare_reg_imm(
          const RegisterX86 reg, const std::uint64_t value);

      std::size_t emit_conditional_jump(
          const Condition condition, const std::int32_t offset);
      std::size_t emit_jump(const std::int32_t offset);
      void emit_jump(const RegisterX86 offset);

      void emit_xmm_binary_op(
          const std::uint8_t opcode,
          const RegisterX86 reg0, const RegisterX86 reg1);

      void emit_move_reg_to_from_mem(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int offset, const bool read);

      void emit_offset(const std::int32_t offset);

      void emit_displacement(const unsigned int displacement);

      template <std::size_t N>
      void emit_immediate(const std::uint64_t value);

      RegisterX86 general_scratch_;
      RegisterX86 stack_size_;
      RegisterX86 float_scratch_;
      AssemblyWrapper func_;
    };


    template <std::size_t N>
    void Assembler<Platform::X86_64>::emit_immediate(const std::uint64_t value)
    {
      if (N == 1) {
        func_.add_byte(static_cast<std::uint8_t>(0xff & value));
        return;
      }

      int i = 0;
      const auto next_byte = [&] {
        return static_cast<std::uint8_t>(0xff & (value >> (8 * (i++))));
      };

      std::array<std::uint8_t, N> bytes;
      std::generate(bytes.begin(), bytes.end(), next_byte);
      func_.add_bytes(bytes.begin(), bytes.end());
    }
  }
}

#endif // LOXX_JIT_ASSEMBLERX86_HPP
