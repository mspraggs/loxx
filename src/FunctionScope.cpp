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
 * Created by Matt Spraggs on 07/05/18.
 */

#include "FunctionScope.hpp"
#include "logging.hpp"
#include "Value.hpp"


namespace loxx
{
  void FunctionScope::add_local(const Token& name)
  {
    for (long int i = locals_.size() - 1; i >= 0; --i) {
      const auto& local = locals_[i];

      if (local.defined and local.depth < scope_depth_) {
        break;
      }
      if (local.name == name.lexeme()) {
        error(name, "Variable with this name already declared in this scope.");
      }
    }
    locals_.push_back({false, false, 0, name.lexeme()});
  }


  void FunctionScope::define_local()
  {
    locals_.back().defined = true;
    locals_.back().depth = scope_depth_;
  }


  Optional<UByteCodeArg> FunctionScope::resolve_local(
      const Token& name, const bool in_function) const
  {
    for (long int i = locals_.size() - 1; i >= 0; --i) {
      if (locals_[i].name == name.lexeme()) {
        if (not in_function and not locals_[i].defined) {
          error(name, "Cannot read local variable in its own initialiser.");
        }
        return static_cast<UByteCodeArg>(i);
      }
    }
    return Optional<UByteCodeArg>();
  }


  Optional <UByteCodeArg> FunctionScope::resolve_upvalue(const Token& name)
  {
    if (enclosing_ == nullptr) {
      // There is only one scope, so we're not going to find an upvalue
      return Optional<UByteCodeArg>();
    }

    auto local = enclosing_->resolve_local(name, true);

    if (local) {
      enclosing_->locals_[*local].is_upvalue = true;
      return add_upvalue(*local, true);
    }

    local = enclosing_->resolve_upvalue(name);
    if (local) {
      return add_upvalue(*local, false);
    }

    return Optional<UByteCodeArg>();
  }


  UByteCodeArg FunctionScope::add_upvalue(const UByteCodeArg index,
                                          const bool is_local)
  {
    for (UByteCodeArg i = 0; i < upvalues_.size(); ++i) {
      if (upvalues_[i].index == index and upvalues_[i].is_local == is_local) {
        return i;
      }
    }

    upvalues_.push_back(Upvalue{is_local, index});
    return static_cast<UByteCodeArg>(upvalues_.size() - 1);
  }


  void FunctionScope::begin_scope()
  {
    ++scope_depth_;
  }


  void FunctionScope::end_scope()
  {
    --scope_depth_;

    while (not locals_.empty() and locals_.back().depth > scope_depth_) {
      if (locals_.back().is_upvalue) {
        add_instruction(Instruction::CloseUpvalue);
      }
      else {
        add_instruction(Instruction::Pop);
      }
      locals_.pop_back();
    }
  }


  Token FunctionScope::make_token(const TokenType type,
                                  std::string lexeme) const
  {
    return Token(type, std::move(lexeme), last_line_num_);
  }


  std::unique_ptr<FunctionScope> FunctionScope::release_enclosing()
  {
    std::unique_ptr<FunctionScope> ret;
    std::swap(ret, enclosing_);
    return ret;
  }


  std::unique_ptr<CodeObject> FunctionScope::release_code_object()
  {
    std::unique_ptr<CodeObject> ret;
    std::swap(ret, code_object_);
    return ret;
  }


  std::vector<FunctionScope::Upvalue> FunctionScope::release_upvalues()
  {
    std::vector<Upvalue> ret;
    std::swap(ret, upvalues_);
    return ret;
  }


  void FunctionScope::add_instruction(const Instruction instruction)
  {
    code_object_->bytecode.push_back(static_cast<std::uint8_t>(instruction));
  }


  void FunctionScope::update_line_num_table(const Token& token)
  {
    // Okay, so I totally ripped off CPython's strategy for encoding line
    // numbers here, but what I can I say? It's a good strategy!

    // The broad idea here is that for each instruction we encode the difference
    // between the last line number number and the current line number, and the
    // last instruction and the current instruction. If the last line number is
    // different to the previous one, the difference in instruction and line is
    // encoded as one or more pairs of bytes.

    int line_num_diff = token.line() - last_line_num_;
    auto line_num_diff_abs = static_cast<unsigned int>(std::abs(line_num_diff));
    std::size_t instr_num_diff =
        code_object_->bytecode.size() - last_instr_num_;

    const auto num_rows =
        std::max(line_num_diff_abs / 128,
                 static_cast<unsigned int>(instr_num_diff / 256));

    if (num_rows > 0) {
      const auto line_num_delta =
          static_cast<std::int8_t>(line_num_diff / num_rows);
      const auto instr_num_delta =
          static_cast<std::int8_t>(instr_num_diff / num_rows);

      for (unsigned int i = 0; i < num_rows; ++i) {
        code_object_->line_num_table.emplace_back(
            line_num_delta, instr_num_delta);
      }

      line_num_diff -= num_rows * line_num_delta;
      instr_num_diff -= num_rows * instr_num_delta;
    }

    code_object_->line_num_table.emplace_back(
        line_num_diff, instr_num_diff);
    last_instr_num_ = code_object_->bytecode.size();
    last_line_num_ = token.line();
  }
}