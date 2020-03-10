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


    std::uint8_t get_reg_rm_bits(const RegisterX86 reg);


    template <>
    class Assembler<Platform::X86_64>
    {
    public:
      using SSAInstruction = SSAInstruction<3>;

      explicit Assembler(const AllocationMap<RegisterX86>& allocation_map);

      AssemblyWrapper assemble(const std::vector<SSAInstruction>& ssa_ir);

    private:
      void insert_type_guards(const ReferenceSet& operands);

      void emit_guard(const Value* location, const ValueType type);

      void emit_add(const SSAInstruction& instruction);
      void emit_move(const SSAInstruction& instruction);
      void emit_multiply(const SSAInstruction& instruction);

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
      template <typename T>
      auto emit_move_reg_imm(
          const RegisterX86 dst, const T value)
          -> typename std::enable_if_t<sizeof(T) == 8, void>;
      template <typename T>
      auto emit_move_reg_imm(
          const RegisterX86 dst, const T value)
          -> typename std::enable_if_t<sizeof(T) < 8, void>;
      void emit_move_reg_imm(
          const RegisterX86 dst, const Value& value);

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

      template <typename T>
      void emit_immediate(const T value);

      Optional<RegisterX86> get_register(const Operand& operand) const;

      const AllocationMap<RegisterX86>* allocation_map_;
      RegisterX86 general_scratch_;
      RegisterX86 stack_size_;
      RegisterX86 float_scratch_;
      AssemblyWrapper func_;
    };


    template <typename T>
    auto Assembler<Platform::X86_64>::emit_move_reg_imm(
        const RegisterX86 dst, const T value)
        -> typename std::enable_if_t<sizeof(T) == 8, void>
    {
      const std::uint8_t rex_prefix =
          reg_is_64_bit(dst) ? 0b01001001 : 0b01001000;
      func_.add_byte(rex_prefix);
      func_.add_byte(0xb8 | get_reg_rm_bits(dst));
      emit_immediate(value);
    }


    template <typename T>
    auto Assembler<Platform::X86_64>::emit_move_reg_imm(
        const RegisterX86 dst, const T value)
        -> typename std::enable_if_t<sizeof(T) < 8, void>
    {
      if (reg_is_64_bit(dst)) {
        func_.add_byte(0x41);
      }
      constexpr std::uint8_t base_opcode = (sizeof(T) == 1) ? 0xb0 : 0xb8;
      func_.add_byte(base_opcode | get_reg_rm_bits(dst));
      emit_immediate(value);
    }


    template <typename T>
    void Assembler<Platform::X86_64>::emit_immediate(const T value)
    {
      const auto begin = reinterpret_cast<const std::uint8_t*>(&value);
      const auto end = begin + sizeof(T);
      func_.add_bytes(begin, end);
    }
  }
}

#endif // LOXX_JIT_ASSEMBLERX86_HPP
