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

#include "../Object.hpp"

#include "AssemblerX86.hpp"
#include "JITError.hpp"
#include "PlatformX86.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    Condition get_inverse_condition(const Condition condition)
    {
      return static_cast<Condition>(static_cast<std::uint8_t>(condition) ^ 1);
    }


    std::uint8_t get_reg_rm_bits(const RegisterX86 reg)
    {
      return static_cast<std::uint8_t>(reg) & 0b111;
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
      return reg_is_64_bit(reg0) | (reg_is_64_bit(reg1) << 2);
    }


    std::uint8_t get_mod_rm_byte_for_regs(
        const RegisterX86 reg0, const RegisterX86 reg1,
        const std::uint8_t offset_bits = 0b11000000)
    {
      return offset_bits | (get_reg_rm_bits(reg0) << 3) | get_reg_rm_bits(reg1);
    }


    Assembler<Platform::X86_64>::Assembler(Trace& trace)
        : trace_(&trace)
    {
      constant_map_.resize(trace_->code_object->constants.size());
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
        else if (reg_supports_ptr(reg) and gen_reg_count == 1) {
          stack_base_ = reg;
          gen_reg_count++;
        }
      }
    }


    void Assembler<Platform::X86_64>::assemble()
    {
      emit_move_reg_imm(stack_base_, trace_->stack_base);

      for (std::size_t pos = 0; pos < trace_->ir_buffer.size(); ++pos) {
        const auto& instruction = trace_->ir_buffer[pos];
        instruction_offsets_.push_back(trace_->assembly.size());
        const auto op = instruction.op();

        switch (op) {

        case Operator::ADD:
          emit_add(pos, instruction);
          break;

        case Operator::CHECK_FALSE:
          emit_condition_guard(instruction, true);
          break;

        case Operator::CHECK_TRUE:
          emit_condition_guard(instruction, false);
          break;

        case Operator::CHECK_TYPE:
          emit_type_guard(instruction);
          break;

        case Operator::DIVIDE:
          break;

        case Operator::EQUAL:
          emit_compare(instruction);
          last_condition_ = Condition::EQUAL;
          break;

        case Operator::JUMP:
          emit_jump(pos, instruction);
          break;

        case Operator::LESS:
          emit_compare(instruction);
          last_condition_ = Condition::BELOW;
          break;

        case Operator::LITERAL:
          emit_literal(pos, instruction);
          break;

        case Operator::LOAD:
          emit_load(pos, instruction);
          break;

        case Operator::LOOP:
          emit_loop(pos, instruction);
          break;

        case Operator::MULTIPLY:
          emit_multiply(pos, instruction);
          break;

        case Operator::LOOP_START:
        case Operator::NOOP:
          // Do nothing.
          break;

        case Operator::PHI:
          emit_phi(instruction);
          break;

        case Operator::POP:
          // Do nothing
          break;

        case Operator::RETURN:
          break;

        case Operator::STORE:
          // Do nothing
          break;

        case Operator::SUBTRACT:
          break;

        default:
#ifndef NDEBUG
          throw JITError("unsupported SSA opcode");
#else
          ;
#endif
        }
      }

      emit_constants();
      trace_->assembly.lock();
      trace_->state = Trace::State::ASSEMBLY_COMPLETE;
    }


    void Assembler<Platform::X86_64>::patch_jumps()
    {
      for (const auto& jump_offset : jump_offsets_) {
        const auto offset_pos = jump_offset.first;
        const auto jump_target_ssa = jump_offset.second;
        const auto jump_target_mcode = instruction_offsets_[jump_target_ssa];
        const std::int32_t offset = jump_target_mcode - offset_pos - 4;
        trace_->assembly.write_integer(offset_pos, offset);
      }
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

      emit_move_reg_mem(
          general_scratch_, stack_base_, get_stack_offset(location));
      emit_compare_reg_imm(general_scratch_, static_cast<std::size_t>(type));

      const auto short_jump_pos = emit_conditional_jump(Condition::EQUAL, 0);
      const auto short_jump_start = trace_->assembly.size();

      const auto exit_address = get_exit_stub_pointer<Platform::X86_64>();
      emit_move_reg_imm(RegisterX86::RAX, true);

      emit_move_reg_imm(general_scratch_, exit_address);
      emit_jump(general_scratch_);
      const auto jump_size = trace_->assembly.size() - short_jump_start;
      trace_->assembly.write_byte(
          short_jump_pos, static_cast<std::uint8_t>(jump_size));
    }


    void Assembler<Platform::X86_64>::emit_add(
        const std::size_t ref, const IRIns& instruction)
    {
      const auto reg0 = get_register(ref);
      const auto& operands = instruction.operands();

      if (operands[0].type() == Operand::Type::IR_REF and
          operands[1].type() == Operand::Type::IR_REF) {
        const auto reg1 = get_register(operands[0]);
        const auto reg2 = get_register(operands[1]);

        if (not reg0 or not reg1 or not reg2) {
          return;
        }

        emit_move_reg_reg(*reg0, *reg1);
        emit_add_reg_reg(*reg0, *reg2);
      }
    }


    void Assembler<Platform::X86_64>::emit_compare(
        const IRIns& instruction)
    {
      const auto& operands = instruction.operands();

      if (operands[0].type() == Operand::Type::IR_REF and
          operands[1].type() == Operand::Type::IR_REF) {
        const auto reg0 = get_register(operands[0]);
        const auto reg1 = get_register(operands[1]);

        if (not reg0 or not reg1) {
          return;
        }

        emit_compare_reg_reg(*reg0, *reg1);
      }
      else if (operands[0].type() == Operand::Type::IR_REF and
          operands[1].is_literal()) {
        const auto reg = get_register(operands[0]);
        if (not reg) {
          return;
        }

        const auto& value = trace_->code_object->constants[operands[1].index()];

        if (holds_alternative<double>(value)) {
          emit_compare_reg_imm(*reg, unsafe_get<double>(value));
        }
        else if (holds_alternative<bool>(value)) {
          emit_compare_reg_imm(*reg, unsafe_get<bool>(value));
        }
        else {
          const auto ptr = unsafe_get<ObjectPtr>(value);
          emit_compare_reg_imm(*reg, reinterpret_cast<std::uint64_t>(ptr));
        }
      }
    }


    void Assembler<Platform::X86_64>::emit_condition_guard(
        const IRIns& instruction, const bool invert_condition)
    {
      if (not last_condition_) {
#ifndef NDEBUG
        throw JITError("invalid assembler state");
#endif
      }
      const auto condition =
          invert_condition ?
          get_inverse_condition(*last_condition_) :
          *last_condition_;
      const auto exit_num = instruction.operand(1).index();
      const auto offset_pos = emit_conditional_jump(condition, 0x01);
      const auto mcode_size = trace_->assembly.size();

      /// TODO: Validate exit num
      emit_push(static_cast<std::uint32_t>(exit_num));
      emit_move_reg_imm(
          general_scratch_, get_exit_stub_pointer<Platform::X86_64>());
      emit_jump(general_scratch_);

      const std::int8_t offset = trace_->assembly.size() - mcode_size;
      trace_->assembly.write_integer(offset_pos, offset);
    }


    void Assembler<Platform::X86_64>::emit_type_guard(
        const IRIns& instruction)
    {
      const auto address = get_stack_address(instruction.operand(0));
      const auto exit_num = instruction.operand(1).index();
      emit_move_reg_mem(
          general_scratch_, stack_base_, get_stack_offset(address));
      emit_compare_reg_imm(
          general_scratch_, static_cast<std::size_t>(instruction.type()));
      const auto offset_pos = emit_conditional_jump(Condition::EQUAL, 0x01);
      const auto mcode_size = trace_->assembly.size();

      /// TODO: Validate exit num
      emit_push(static_cast<std::uint32_t>(exit_num));
      emit_move_reg_imm(
          general_scratch_, get_exit_stub_pointer<Platform::X86_64>());
      emit_jump(general_scratch_);

      const std::int8_t offset = trace_->assembly.size() - mcode_size;
      trace_->assembly.write_integer(offset_pos, offset);
    }


    void Assembler<Platform::X86_64>::emit_jump(
        const std::size_t pos, const IRIns& instruction)
    {
      const auto& operands = instruction.operands();
      const auto offset = operands[0].index();
      const auto jump_target = pos + 1 + offset;
      const auto offset_pos = emit_jump(0x100);
      jump_offsets_.emplace_back(std::make_pair(offset_pos, jump_target));
    }


    void Assembler<Platform::X86_64>::emit_loop(
        const std::size_t pos, const IRIns& instruction)
    {
      const auto& operands = instruction.operands();
      const auto offset = operands[0].index();
      const auto jump_target = pos + 1 - offset;
      const auto offset_pos = emit_jump(0x100);

      const auto assembly_jump_target = instruction_offsets_[jump_target];
      const std::int32_t assembly_offset =
          assembly_jump_target - offset_pos - sizeof(std::int32_t);
      trace_->assembly.write_integer(offset_pos, assembly_offset);
    }


    void Assembler<Platform::X86_64>::emit_literal(
        const std::size_t pos, const IRIns& instruction)
    {
      const auto dst_reg = get_register(pos);

      if (not dst_reg) {
#ifndef NDEBUG
        throw JITError("no destination register for literal");
#endif
      }

      const auto index = instruction.operand(0).index();
      const auto& value = trace_->code_object->constants[index];
      if (holds_alternative<double>(value)) {
        const auto offset_pos = emit_move_reg_mem(
            *dst_reg, RegisterX86::RIP, 0x100);
        constant_map_[index].emplace_back(
            offset_pos, trace_->assembly.size());
      }
      else if (holds_alternative<bool>(value)) {
        emit_move_reg_imm(*dst_reg, unsafe_get<bool>(value));
      }
      else if (holds_alternative<ObjectPtr>(value)) {
        const auto offset_pos = emit_move_reg_mem(
            *dst_reg, RegisterX86::RIP, 0x100);
        constant_map_[index].emplace_back(offset_pos, trace_->assembly.size());
      }
    }


    void Assembler<Platform::X86_64>::emit_load(
        const std::size_t pos, const IRIns& instruction)
    {
      const auto dst_reg = get_register(pos);
      const auto address = get_stack_address(instruction.operand(0));

      if (not dst_reg) {
        return;
      }

      if (not address) {
#ifndef NDEBUG
        throw JITError("invalid source address for load");
#endif
      }

      emit_move_reg_mem(*dst_reg, stack_base_, get_stack_offset(address) + 8);
    }


    void Assembler<Platform::X86_64>::emit_phi(const IRIns& instruction)
    {
      const auto dst_reg = get_register(instruction.operand(0));
      const auto src_reg = get_register(instruction.operand(1));

      if (not src_reg or not dst_reg) {
#ifndef NDEBUG
        throw JITError(
            "invalid source register or destination address for store");
#endif
      }

      emit_move_reg_reg(*dst_reg, *src_reg);
    }


    void Assembler<Platform::X86_64>::emit_store(
        const IRIns& instruction)
    {
      const auto address = get_stack_address(instruction.operand(0));
      const auto src_reg = get_register(instruction.operand(1));

      if (not src_reg or not address) {
#ifndef NDEBUG
        throw JITError(
            "invalid source register or destination address for store");
#endif
      }

      emit_move_reg_imm(general_scratch_, address);
      emit_move_mem_reg(general_scratch_, *src_reg, 8);
    }


    void Assembler<Platform::X86_64>::emit_constants()
    {
      for (std::size_t idx = 0; idx < constant_map_.size(); ++idx) {
        const auto& mappings = constant_map_[idx];
        if (mappings.size() == 0) {
          continue;
        }

        const auto constant_pos = trace_->assembly.size();
        const auto& constant = trace_->code_object->constants[idx];

        if (holds_alternative<double>(constant)) {
          emit_immediate(unsafe_get<double>(constant));
        }
        else if (holds_alternative<bool>(constant)) {
          emit_immediate(unsafe_get<bool>(constant));
        }
        else if (holds_alternative<ObjectPtr>(constant)) {
          emit_immediate(unsafe_get<ObjectPtr>(constant));
        }

        for (const auto& mapping : mappings) {
          const auto offset = static_cast<std::uint32_t>(
              constant_pos - mapping.second);
          trace_->assembly.write_integer(mapping.first, offset);
        }
      }
    }


    void Assembler<Platform::X86_64>::emit_multiply(
        const std::size_t ref, const IRIns& instruction)
    {
      const auto reg0 = get_register(ref);
      const auto& operands = instruction.operands();

      if (operands[0].type() == Operand::Type::IR_REF and
          operands[1].type() == Operand::Type::IR_REF) {
        const auto reg1 = get_register(operands[0]);
        const auto reg2 = get_register(operands[1]);

        if (not reg0 or not reg1 or not reg2) {
          return;
        }

        emit_move_reg_reg(*reg0, *reg1);
        emit_multiply_reg_reg(*reg0, *reg2);
      }
    }


    void Assembler<Platform::X86_64>::emit_return()
    {
      trace_->assembly.add_byte(0xc3);
    }


    void Assembler<Platform::X86_64>::emit_push(const RegisterX86 src)
    {
      if (reg_is_64_bit(src)) {
        trace_->assembly.add_byte(0x41);
      }
      trace_->assembly.add_byte(0x50 | get_reg_rm_bits(src));
    }


    void Assembler<Platform::X86_64>::emit_push(const std::uint32_t value)
    {
      trace_->assembly.add_byte(0x68);
      emit_immediate(value);
    }


    void Assembler<Platform::X86_64>::emit_pop(const RegisterX86 dst)
    {
      if (reg_is_64_bit(dst)) {
        trace_->assembly.add_byte(0x41);
      }
      trace_->assembly.add_byte(0x58 | get_reg_rm_bits(dst));
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
#ifndef NDEBUG
        throw JITError("invalid registers for add");
#endif
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
#ifndef NDEBUG
        throw JITError("invalid registers for add");
#endif
      }
    }


    void Assembler<Platform::X86_64>::emit_decrement(const RegisterX86 reg)
    {
      if (reg_supports_float(reg)) {
#ifndef NDEBUG
        throw JITError("invalid register for decrement");
#endif
      }
      const std::uint8_t rex_prefix =
          0b01001000 | get_rex_prefix_for_regs(reg, RegisterX86::RAX);
      const std::uint8_t mod_rm_byte = 0b11001000 | get_reg_rm_bits(reg);

      trace_->assembly.add_bytes(rex_prefix, 0xff, mod_rm_byte);
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
        const std::uint8_t mod_rm_byte = get_mod_rm_byte_for_regs(src, dst);
        trace_->assembly.add_byte(rex_prefix);
        trace_->assembly.add_byte(0x89);
        trace_->assembly.add_byte(mod_rm_byte);
      }
      else if (reg_supports_float(src) and reg_supports_float(dst)) {
        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(src, dst);
        const auto mod_rm_byte = get_mod_rm_byte_for_regs(dst, src);

        trace_->assembly.add_byte(0xf2);
        if (rex_prefix_reg_bits != 0) {
          trace_->assembly.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        trace_->assembly.add_bytes(0x0f, 0x10, mod_rm_byte);
      }
      else if (reg_supports_ptr(src) and reg_supports_float(dst)) {
        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(src, dst);
        const auto mod_rm_byte = get_mod_rm_byte_for_regs(dst, src);

        trace_->assembly.add_bytes(
            0x66, 0x48 | rex_prefix_reg_bits, 0x0f, 0x6e, mod_rm_byte);
      }
      else {
#ifndef NDEBUG
        throw JITError("invalid move registers");
#endif
      }
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_reg_mem(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int displacement)
    {
      return emit_move_reg_to_from_mem(dst, src, displacement, true);
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_reg_mem(
        const RegisterX86 dst, const Value* src,
        const unsigned int displacement)
    {
      return emit_move_reg_to_from_mem(dst, src, displacement, true);
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_mem_reg(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int displacement)
    {
      return emit_move_reg_to_from_mem(dst, src, displacement, false);
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_mem_reg(
        const RegisterX86 dst, const Value* src,
        const unsigned int displacement)
    {
      return emit_move_reg_to_from_mem(dst, src, displacement, false);
    }


    void Assembler<Platform::X86_64>::emit_compare_reg_reg(
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      if (reg_supports_float(reg0) and reg_supports_float(reg1)) {
        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(reg1, reg0);
        const auto mod_rm_byte = get_mod_rm_byte_for_regs(reg0, reg1);

        trace_->assembly.add_byte(0x66);
        if (rex_prefix_reg_bits != 0) {
          trace_->assembly.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        trace_->assembly.add_bytes(0x0f, 0x2e, mod_rm_byte);
      }
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
#ifndef NDEBUG
          throw JITError("unsigned integer overflow");
#endif
        }
      } ();

      const std::uint8_t mod_rm_byte = 0b11111000 | get_reg_rm_bits(reg);

      trace_->assembly.add_byte(rex_prefix);
      trace_->assembly.add_byte(opcode);
      trace_->assembly.add_byte(mod_rm_byte);

      if (value > std::numeric_limits<std::uint8_t>::max()) {
        emit_immediate(static_cast<std::uint32_t>(value));
      }
      else if (value >= 0) {
        emit_immediate(static_cast<std::uint8_t>(value));
      }
    }


    void Assembler<Platform::X86_64>::emit_move_reg_imm(
        const RegisterX86 dst, const Operand& operand)
    {
      if (operand.type() != Operand::Type::LITERAL) {
        return;
      }
      const auto& value = trace_->code_object->constants[operand.index()];
      if (holds_alternative<double>(value)) {
        emit_move_reg_imm(general_scratch_, unsafe_get<double>(value));
        emit_move_reg_reg(dst, general_scratch_);
      }
      else if (holds_alternative<bool>(value)) {
        emit_move_reg_imm(dst, unsafe_get<bool>(value));
      }
      else if (holds_alternative<ObjectPtr>(value)) {
        emit_move_reg_imm(dst, unsafe_get<ObjectPtr>(value));
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
        trace_->assembly.add_byte(0x0f);
      }
      trace_->assembly.add_byte(opcode);

      const auto ret = trace_->assembly.size();
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

      trace_->assembly.add_byte(opcode);

      const auto ret = trace_->assembly.size();
      emit_offset(offset);

      return ret;
    }


    void Assembler<Platform::X86_64>::emit_jump(const RegisterX86 offset)
    {
      if (reg_is_64_bit(offset)) {
        trace_->assembly.add_byte(0x41);
      }
      trace_->assembly.add_byte(0xff);
      trace_->assembly.add_byte(0xe0 | get_reg_rm_bits(offset));
    }


    void Assembler<Platform::X86_64>::emit_xmm_binary_op(
        const std::uint8_t opcode,
        const RegisterX86 reg0, const RegisterX86 reg1)
    {
      const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(reg1, reg0);
      const auto mod_rm_byte = get_mod_rm_byte_for_regs(reg0, reg1);

      trace_->assembly.add_byte(0xf2);
      if (rex_prefix_reg_bits != 0) {
        trace_->assembly.add_bytes(0x40 | rex_prefix_reg_bits);
      }
      trace_->assembly.add_bytes(0x0f, opcode, mod_rm_byte);
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_reg_to_from_mem(
        const RegisterX86 dst, const RegisterX86 src,
        const unsigned int offset, const bool read)
    {
      if (reg_supports_ptr(src) and reg_supports_ptr(dst)) {
        const std::uint8_t offset_bits =
            src != RegisterX86::RIP ? get_offset_bits(offset) : 0;
        const std::uint8_t rex_prefix =
            0b01001000 | get_rex_prefix_for_regs(src, dst);
        const std::uint8_t mod_rm_byte =
            get_mod_rm_byte_for_regs(dst, src, offset_bits);
        trace_->assembly.add_bytes(rex_prefix, read ? 0x8b : 0x89, mod_rm_byte);
        return emit_displacement(offset);
      }
      else if (
          (reg_supports_float(dst) and read) or
          (reg_supports_float(src) and not read)) {
        const auto reg0 = read ? src : dst;
        const auto reg1 = read ? dst : src;
        const std::uint8_t offset_bits =
            reg0 != RegisterX86::RIP ? get_offset_bits(offset) : 0;

        const auto rex_prefix_reg_bits = get_rex_prefix_for_regs(reg0, reg1);
        const auto mod_rm_byte =
            get_mod_rm_byte_for_regs(reg1, reg0, offset_bits);

        trace_->assembly.add_byte(0xf2);
        if (rex_prefix_reg_bits != 0) {
          trace_->assembly.add_bytes(0x40 | rex_prefix_reg_bits);
        }
        trace_->assembly.add_bytes(0x0f, read ? 0x10 : 0x11, mod_rm_byte);

        return emit_displacement(offset);
      }
      else {
#ifndef NDEBUG
        throw JITError("invalid move registers");
#endif
      }
    }


    std::size_t Assembler<Platform::X86_64>::emit_move_reg_to_from_mem(
        const RegisterX86 reg, const Value* address,
        const unsigned int displacement, const bool read)
    {
      const auto is_float = reg_supports_float(reg);
      const std::uint8_t rex_prefix = [&] {
        if (is_float) {
          return reg_is_64_bit(reg) * 0x44;
        }
        else {
          return 0b01001000 | (reg_is_64_bit(reg) << 2);
        }
      } ();

      const std::uint8_t opcode = [&] {
        if (is_float) {
          return 0x10 | (not read);
        }
        else {
          return 0x89 | (read << 1);
        }
      } ();
      const std::uint8_t mod_rm_byte = (get_reg_rm_bits(reg) << 3) | 0b0100;
      const std::uint8_t sib_byte = 0x25;

      if (is_float) {
        trace_->assembly.add_byte(0xf2);
        if (rex_prefix != 0) {
          trace_->assembly.add_byte(rex_prefix);
        }
        trace_->assembly.add_bytes(
            {0x0f, opcode, mod_rm_byte, sib_byte});
      }
      else {
        trace_->assembly.add_bytes(
            {rex_prefix, opcode, mod_rm_byte, sib_byte});
      }

      const auto ret = trace_->assembly.size();
      auto value = reinterpret_cast<std::uint64_t>(address) + displacement;
      emit_immediate(static_cast<std::uint32_t>(value));
      return ret;
    }


    void Assembler<Platform::X86_64>::emit_offset(const std::int32_t offset)
    {
      const auto is_short = offset <= 127 and offset >= -128;
      if (is_short) {
        emit_immediate(static_cast<std::uint8_t>(offset));
      }
      else {
        emit_immediate((offset));
      }
    }


    std::size_t Assembler<Platform::X86_64>::emit_displacement(
        const unsigned int displacement)
    {
      const auto ret = trace_->assembly.size();
      if (displacement > std::numeric_limits<std::uint8_t>::max()) {
        emit_immediate(static_cast<std::uint32_t>(displacement));
      }
      else if (displacement > 0) {
        emit_immediate(static_cast<std::uint8_t>(displacement));
      }
      return ret;
    }


    Optional<RegisterX86> Assembler<Platform::X86_64>::get_register(
        const std::size_t ref) const
    {
      const auto& allocation = trace_->allocation_map[ref];

      if (allocation and holds_alternative<RegisterX86>(*allocation)) {
        return unsafe_get<RegisterX86>(*allocation);
      }

      return {};
    }


    Optional<RegisterX86> Assembler<Platform::X86_64>::get_register(
        const Operand& operand) const
    {
      if (operand.type() != Operand::Type::IR_REF) {
        return {};
      }

      return get_register(operand.index());
    }


    const Value* Assembler<Platform::X86_64>::get_stack_address(
        const Operand& operand) const
    {
      if (operand.type() != Operand::Type::STACK_REF) {
        return nullptr;
      }

      const auto idx = operand.index();
      return trace_->stack_base + idx;
    }


    unsigned int Assembler<Platform::X86_64>::get_stack_offset(
        const Value* ptr) const
    {
      return std::distance(trace_->stack_base, ptr) * sizeof(Value);
    }


    AssemblyWrapper& Assembler<Platform::X86_64>::assembly()
    {
      return trace_->assembly;
    }


    const AssemblyWrapper& Assembler<Platform::X86_64>::assembly() const
    {
      return trace_->assembly;
    }
  }
}
