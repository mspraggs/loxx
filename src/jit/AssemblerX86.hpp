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
    class Assembler<RegisterX86>
    {
    public:
      Assembler();

      AssemblyFunction assemble(
          const std::vector<SSAInstruction<2>>& ssa_ir,
          const AllocationMap<RegisterX86>& allocation_map,
          const OperandSet& external_operands);

    private:
      void insert_type_guards(const OperandSet& operands);

      void add_addition(
          const SSAInstruction<2>& instruction,
          const AllocationMap<RegisterX86>& allocation_map);
      void add_move(
          const SSAInstruction<2>& instruction,
          const AllocationMap<RegisterX86>& allocation_map);
      void add_multiplication(
          const SSAInstruction<2>& instruction,
          const AllocationMap<RegisterX86>& allocation_map);

      void add_return();
      void add_push(const RegisterX86 src);
      void add_pop(const RegisterX86 dst);

      void add_addition_reg_reg(
          const RegisterX86 reg0, const RegisterX86 reg1);
      void add_multiplication_reg_reg(
          const RegisterX86 reg0, const RegisterX86 reg1);

      void add_decrement(const RegisterX86 reg);

      void add_move_reg_reg(const RegisterX86 dst, const RegisterX86 src);
      void add_move_reg_mem(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int offset = 0);
      void add_move_mem_reg(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int offset = 0);
      void add_move_reg_imm(
          const RegisterX86 dst, const std::uint64_t value);

      void add_compare_reg_imm(
          const RegisterX86 reg, const std::uint64_t value);

      std::size_t add_conditional_jump(
          const Condition condition, std::int32_t offset);
      std::size_t add_jump(const std::int32_t offset);
      void add_jump(const RegisterX86 offset);

      void add_xmm_binary_op(
          const std::uint8_t opcode,
          const RegisterX86 reg0, const RegisterX86 reg1);

      void add_move_reg_to_from_mem(
          const RegisterX86 dst, const RegisterX86 src,
          const unsigned int offset, const bool read);

      template <std::size_t N>
      void add_immediate(const std::uint64_t value);

      RegisterX86 general_scratch_;
      RegisterX86 float_scratch_;
      AssemblyFunction func_; 
    };


    template <std::size_t N>
    void Assembler<RegisterX86>::add_immediate(const std::uint64_t value)
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
