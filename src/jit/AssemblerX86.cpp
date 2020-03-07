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
#include "PlatformX86.hpp"


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


    std::uint8_t get_offset_bits(const unsigned int offset)
    {
      if (offset == 0) {
        return 0b00000000;
      }
      if (offset <= std::numeric_limits<std::uint8_t>::max()) {
        return 0b01000000;
      }
      return 0b10000000;
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


    Assembler<Platform::X86_64>::Assembler()
    {
      const auto scratch_registers =
          get_scratch_registers<Platform::X86_64>();

      unsigned int gen_reg_count = 0;
      for (const auto reg : scratch_registers) {
        if (reg_supports_float(reg)) {
          float_scratch_ = reg;
        }
        else if (reg_supports_ptr(reg) and gen_reg_count == 0) {
          general_scratch_ = reg;
          gen_reg_count++;
        }
        else {
          stack_size_ = reg;
        }
      }
    }


    AssemblyWrapper Assembler<Platform::X86_64>::assemble(
        const std::vector<SSAInstruction>& ssa_ir,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      emit_move_reg_mem(stack_size_, general_scratch_);

      for (const auto& instruction : ssa_ir) {
        const auto op = instruction.op();

        switch (op) {

        case Operator::ADD:
          emit_add(instruction, allocation_map);
          break;
        case Operator::DIVIDE:
          break;
        case Operator::MOVE:
          emit_move(instruction, allocation_map);
          break;
        case Operator::MULTIPLY:
          emit_multiply(instruction, allocation_map);
          break;
        case Operator::POP:
          emit_decrement(stack_size_);
          break;
        case Operator::RETURN:
          break;
        case Operator::SUBTRACT:
          break;
        default:
          throw JITError("unsupported SSA opcode");
        }
      }

      const auto exit_location = get_exit_function_pointer<Platform::X86_64>();
      emit_move_reg_imm(
          general_scratch_, reinterpret_cast<std::uint64_t>(exit_location));
      emit_move_reg_imm(RegisterX86::RAX, 1);
      emit_jump(general_scratch_);
      return std::move(func_);
    }


    void Assembler<Platform::X86_64>::insert_type_guards(
        const ReferenceSet& references)
    {
      // The aim here is to generate assembly to check each operand in the
      // supplied set against the expected type. If any of these checks returns
      // false, the function should immediately return.
      //
      // Resulting assembly should look a bit like this:
      //
      //     mov     rax, [$op0_address] ; Offset by eight bytes
      //     cmp     rax, $op0_type
      //     jne     quit
      //     mov     rax, [$op1_address]
      //     cmp     rax, $op1_type
      //     jne     quit
      //     ...
      //     jmp     start
      // quit:
      //     mov     rax, 1
      //     ret
      // start:
      //     ...     ; Function body here
      //     mov     rax, 0
      //     ret

      std::vector<std::int32_t> jump_offsets;
      jump_offsets.reserve(references.size());
      std::vector<std::int32_t> jump_starts;
      jump_starts.reserve(references.size());

      for (const auto& reference : references) {
        emit_move_reg_imm(
            general_scratch_,
            reinterpret_cast<std::uint64_t>(reference));
        emit_move_reg_mem(general_scratch_, general_scratch_);
        emit_compare_reg_imm(
            general_scratch_, static_cast<std::uint64_t>(reference->index()));

        /// TODO: Should be able to precompute all this given the input.
        jump_offsets.push_back(
            emit_conditional_jump(Condition::NOT_EQUAL, 256));
        jump_starts.push_back(func_.size());
      }

      const auto start_jump_pos = emit_jump(0);
      const auto start_jump_size = func_.size();

      for (std::size_t i = 0; i < references.size(); ++i) {
        const auto pos = jump_offsets[i];
        const auto jump_size =
            static_cast<std::int32_t>(func_.size() - jump_starts[i]);
        func_.write_integer(pos, jump_size);
      }

      emit_move_reg_imm(RegisterX86::RAX, 1);
      emit_pop(RegisterX86::RBP);
      emit_return();

      func_.write_byte(
          start_jump_pos,
          static_cast<std::uint8_t>(func_.size() - start_jump_size));
    }


    void Assembler<Platform::X86_64>::emit_guard(
        const Value* location, const ValueType type)
    {
      // Here we want to check that the value at the specified location has the
      // given type. The resulting assembly should look like this:
      //
      //     mov     rax, [location]
      //     cmp     rax, type
      //     je      ok
      //     jmp     not_okay
      // ok:
      //     ...
      // not_okay:
      //     ???

      emit_move_reg_imm(
          general_scratch_, reinterpret_cast<std::uint64_t>(location));
      emit_move_reg_mem(general_scratch_, general_scratch_);
      emit_compare_reg_imm(general_scratch_, static_cast<std::size_t>(type));

      const auto short_jump_pos = emit_conditional_jump(Condition::EQUAL, 0);
      const auto short_jump_start = func_.size();

      const auto exit_address = get_exit_function_pointer<Platform::X86_64>();
      emit_move_reg_imm(
          general_scratch_, reinterpret_cast<std::uint64_t>(exit_address));
      emit_move_reg_imm(RegisterX86::RAX, 1);

      const auto jump_size = func_.size() - short_jump_start;
      func_.write_byte(short_jump_pos, static_cast<std::uint8_t>(jump_size));
    }


    void Assembler<Platform::X86_64>::emit_add(
        const SSAInstruction& instruction,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      const auto& operands = instruction.operands();

      if (holds_alternative<VirtualRegister>(operands[0]) and
          holds_alternative<VirtualRegister>(operands[1]) and
          holds_alternative<VirtualRegister>(operands[2])) {
        const auto reg0 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[0])));
        const auto reg1 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[1])));
        const auto reg2 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[2])));
        emit_move_reg_reg(reg0, reg1);
        emit_add_reg_reg(reg0, reg2);
      }
    }


    void Assembler<Platform::X86_64>::emit_move(
        const SSAInstruction& instruction,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      const auto& operands = instruction.operands();

      if (holds_alternative<VirtualRegister>(operands[0]) and
          holds_alternative<const Value*>(operands[1])) {
        const auto address = unsafe_get<const Value*>(operands[1]);
        const auto address_raw = reinterpret_cast<std::uint64_t>(address);
        const auto dst_reg = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[0])));

        emit_guard(address, static_cast<ValueType>(address->index()));
        emit_move_reg_imm(general_scratch_, address_raw);
        emit_move_reg_mem(dst_reg, general_scratch_, 8);
      }
      else if (holds_alternative<VirtualRegister>(operands[0]) and
          holds_alternative<VirtualRegister>(operands[1])) {
        const auto src_reg = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[1])));
        const auto dst_reg = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[0])));
        emit_move_reg_reg(dst_reg, src_reg);
      }
      else if (holds_alternative<const Value*>(operands[0]) and
          holds_alternative<VirtualRegister>(operands[1])) {
        const auto address = unsafe_get<const Value*>(operands[0]);
        const auto address_raw = reinterpret_cast<std::uint64_t>(address);
        const auto src_reg = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[1])));

        emit_guard(address, static_cast<ValueType>(address->index()));
        emit_move_reg_imm(general_scratch_, address_raw);
        emit_move_mem_reg(general_scratch_, src_reg, 8);
      }
    }


    void Assembler<Platform::X86_64>::emit_multiply(
        const SSAInstruction& instruction,
        const AllocationMap<RegisterX86>& allocation_map)
    {
      const auto& operands = instruction.operands();

      if (holds_alternative<VirtualRegister>(operands[0]) and
          holds_alternative<VirtualRegister>(operands[1]) and
          holds_alternative<VirtualRegister>(operands[2])) {
        const auto reg0 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[0])));
        const auto reg1 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[1])));
        const auto reg2 = unsafe_get<RegisterX86>(
            allocation_map.at(unsafe_get<VirtualRegister>(operands[2])));
        emit_move_reg_reg(reg0, reg1);
        emit_multiply_reg_reg(reg0, reg2);
      }
    }


    void Assembler<Platform::X86_64>::emit_return()
    {
      func_.add_byte(0xc3);
    }


    void Assembler<Platform::X86_64>::emit_push(const RegisterX86 src)
    {
      func_.add_byte(0x50 | get_reg_rm_bits(src));
    }


    void Assembler<Platform::X86_64>::emit_pop(const RegisterX86 dst)
    {
      func_.add_byte(0x58 | get_reg_rm_bits(dst));
    }


    void Assembler<Platform::X86_64>::emit_add_reg_reg(
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      if (reg_supports_float(reg0) and reg_supports_float(reg1)) {
        emit_xmm_binary_op(0x58, reg0, reg1);
      }
      else if (reg_supports_int(reg0) and reg_supports_int(reg1)) {

      }
      else {
        throw JITError("invalid registers for add");
      }
    }


    void Assembler<Platform::X86_64>::emit_multiply_reg_reg(
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      if (reg_supports_float(reg0) and reg_supports_float(reg1)) {
        emit_xmm_binary_op(0x59, reg0, reg1);
      }
      else if (reg_supports_int(reg0) and reg_supports_int(reg1)) {

      }
      else {
        throw JITError("invalid registers for add");
      }
    }


    void Assembler<Platform::X86_64>::emit_decrement(const RegisterX86 reg)
    {
      if (reg_supports_float(reg)) {
        throw JITError("invalid register for decrement");
      }
      const std::uint8_t rex_prefix =
          0b01001000 | get_rex_prefix_for_regs(reg, RegisterX86::RAX);
      const std::uint8_t mod_rm_byte = 0b11001000 | get_reg_rm_bits(reg);

      func_.add_bytes(rex_prefix, 0xff, mod_rm_byte);
    }


    void Assembler<Platform::X86_64>::emit_move_reg_reg(
        const RegisterX86 dst, const RegisterX86 src)
    {
      if (src == dst) {
        return;
      }
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
        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(src, dst);
        const auto mod_rm_byte =
            0b11000000 | (get_reg_rm_bits(dst) << 3) | get_reg_rm_bits(src);

        func_.add_byte(0xf2);
        if (rex_prefix_reg_bits != 0) {
          func_.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        func_.add_bytes(0x0f, 0x10, mod_rm_byte);
      }
      else {
        throw JITError("invalid move registers");
      }
    }


    void Assembler<Platform::X86_64>::emit_move_reg_mem(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int displacement)
    {
      if (reg_supports_ptr(dst)) {
        emit_move_reg_to_from_mem(dst, src, displacement, true);
      }
      else if (reg_supports_float(dst)) {
        const std::uint8_t offset_bits = get_offset_bits(displacement);

        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(src, dst);
        const auto mod_rm_byte =
            offset_bits | (get_reg_rm_bits(dst) << 3) | get_reg_rm_bits(src);

        func_.add_byte(0xf2);
        if (rex_prefix_reg_bits != 0) {
          func_.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        func_.add_bytes(0x0f, 0x10, mod_rm_byte);

        emit_displacement(displacement);
      }
      else {
        throw JITError("invalid move registers");
      }
    }


    void Assembler<Platform::X86_64>::emit_move_mem_reg(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int displacement)
    {
      if (reg_supports_ptr(src)) {
        emit_move_reg_to_from_mem(dst, src, displacement, false);
      }
      else if (reg_supports_float(src)) {
        const std::uint8_t offset_bits = get_offset_bits(displacement);

        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(dst, src);
        const auto mod_rm_byte =
            offset_bits | (get_reg_rm_bits(src) << 3) | get_reg_rm_bits(dst);

        func_.add_byte(0xf2);
        if (rex_prefix_reg_bits != 0) {
          func_.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        func_.add_bytes(0x0f, 0x11, mod_rm_byte);

        emit_displacement(displacement);
      }
      else {
        throw JITError("invalid move registers");
      }
    }



    void Assembler<Platform::X86_64>::emit_move_reg_imm(
        const RegisterX86 dst, const std::uint64_t value)
    {
      const std::uint8_t rex_prefix =
          reg_is_64_bit(dst) ? 0b01001001 : 0b01001000;
      func_.add_byte(rex_prefix);
      func_.add_byte(0xb8 | get_reg_rm_bits(dst));
      emit_immediate<8>(value);
    }


    void Assembler<Platform::X86_64>::emit_compare_reg_imm(
        const RegisterX86 reg, const std::uint64_t value)
    {
      const std::uint8_t rex_prefix =
          reg_is_64_bit(reg) ? 0b01001001 : 0b01001000;

      const std::uint8_t opcode = [&] {
        if (value < std::numeric_limits<std::uint8_t>::max()) {
          return 0x83;
        }
        else if (value < std::numeric_limits<std::uint32_t>::max()) {
          return 0x81;
        }
        else {
          throw JITError("unsigned integer overflow");
        }
      } ();

      const std::uint8_t mod_rm_byte =
          0b11111000 | get_reg_rm_bits(reg);

      func_.add_byte(rex_prefix);
      func_.add_byte(opcode);
      func_.add_byte(mod_rm_byte);

      if (value > std::numeric_limits<std::uint8_t>::max()) {
        emit_immediate<4>(value);
      }
      else if (value >= 0) {
        emit_immediate<1>(value);
      }
    }


    std::size_t Assembler<Platform::X86_64>::emit_conditional_jump(
        const Condition condition, std::int32_t offset)
    {
      const auto is_short = offset <= 127 and offset >= -128;

      const std::uint8_t opcode = [&] {
        const auto near_offset = is_short ? 0x00 : 0x10;
        switch (condition) {
        case Condition::ABOVE:
          return near_offset + 0x77;
        case Condition::ABOVE_OR_EQUAL:
          return near_offset + 0x73;
        case Condition::BELOW:
          return near_offset + 0x72;
        case Condition::BELOW_OR_EQUAL:
          return near_offset + 0x76;
        case Condition::EQUAL:
          return near_offset + 0x74;
        case Condition::NOT_EQUAL:
          return near_offset + 0x75;
        case Condition::GREATER:
          return near_offset + 0x7f;
        case Condition::GREATER_OR_EQUAL:
          return near_offset + 0x7d;
        case Condition::LESS:
          return near_offset + 0x7c;
        case Condition::LESS_OR_EQUAL:
          return near_offset + 0x7e;
        }
      } ();

      if (not is_short) {
        func_.add_byte(0x0f);
      }
      func_.add_byte(opcode);

      const auto ret = func_.size();
      emit_offset(offset);

      return ret;
    }


    std::size_t Assembler<Platform::X86_64>::emit_jump(const std::int32_t offset)
    {
      const auto is_short = offset <= 127 and offset >= -128;

      const std::uint8_t opcode = [&] {
        if (is_short) {
          return 0xeb;
        }
        return 0xe9;
      } ();

      func_.add_byte(opcode);

      const auto ret = func_.size();
      emit_offset(offset);

      return ret;
    }


    void Assembler<Platform::X86_64>::emit_jump(const RegisterX86 offset)
    {
      if (reg_is_64_bit(offset)) {
        func_.add_byte(0x41);
      }
      func_.add_byte(0xff);
      func_.add_byte(0xe0 | get_reg_rm_bits(offset));
    }


    void Assembler<Platform::X86_64>::emit_xmm_binary_op(
        const std::uint8_t opcode,
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(reg1, reg0);
      const auto mod_rm_byte =
          0b11000000 | (get_reg_rm_bits(reg0) << 3) | get_reg_rm_bits(reg1);

      func_.add_byte(0xf2);
      if (rex_prefix_reg_bits != 0) {
        func_.add_bytes(0x40 | rex_prefix_reg_bits);
      }
      func_.add_bytes(0x0f, opcode, mod_rm_byte);
    }


    void Assembler<Platform::X86_64>::emit_move_reg_to_from_mem(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int offset, const bool read)
    {
      const std::uint8_t offset_bits = get_offset_bits(offset);

      const std::uint8_t rex_prefix =
          0b01001000 | get_rex_prefix_for_regs(src, dst);
      const std::uint8_t mod_rm_byte =
          offset_bits | (get_reg_rm_bits(dst) << 3) | get_reg_rm_bits(src);
      func_.add_byte(rex_prefix);
      func_.add_byte(read ? 0x8b : 0x89);
      func_.add_byte(mod_rm_byte);

      if (offset > std::numeric_limits<std::uint8_t>::max()) {
        emit_immediate<4>(offset);
      }
      else if (offset > 0) {
        emit_immediate<1>(offset);
      }
    }


    void Assembler<Platform::X86_64>::emit_offset(const std::int32_t offset)
    {
      const auto is_short = offset <= 127 and offset >= -128;
      if (is_short) {
        emit_immediate<1>(static_cast<std::uint64_t>(offset));
      }
      else {
        emit_immediate<4>(static_cast<std::uint64_t>(offset));
      }
    }


    void Assembler<Platform::X86_64>::emit_displacement(
        const unsigned int displacement)
    {
      if (displacement > std::numeric_limits<std::uint8_t>::max()) {
        emit_immediate<4>(displacement);
      }
      else if (displacement > 0) {
        emit_immediate<1>(displacement);
      }
    }
  }
}
