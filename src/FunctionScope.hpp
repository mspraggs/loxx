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

#ifndef LOXX_FUNCTIONSCOPE_HPP
#define LOXX_FUNCTIONSCOPE_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "CodeObject.hpp"
#include "globals.hpp"
#include "Instruction.hpp"
#include "Optional.hpp"
#include "Token.hpp"


namespace loxx
{
  enum class FunctionType {
    Function,
    Initialiser,
    Method,
    None
  };


  class FunctionScope
  {
  public:
    struct Upvalue;

    explicit FunctionScope(const FunctionType type,
                           std::unique_ptr<FunctionScope> enclosing = nullptr)
        : type_(type), last_line_num_(0), last_instr_num_(0),
          scope_depth_(enclosing == nullptr ? 0 : enclosing->scope_depth_ + 1),
          enclosing_(std::move(enclosing)), code_object_(new CodeObject)
    {
      if (type_ == FunctionType::Function) {
        locals_.push_back(Local{false, false, 0, ""});
      }
    }

    void declare_local(const Token& name);
    void define_local();
    void add_local(const Token& name);

    Optional<UByteCodeArg> resolve_local(
        const Token& name, const bool in_function) const;
    Optional <UByteCodeArg> resolve_upvalue(const Token& name);
    UByteCodeArg add_upvalue(const UByteCodeArg index, const bool is_local);

    UByteCodeArg add_named_constant(const std::string& lexeme,
                                    const Value& value);
    UByteCodeArg add_string_constant(const std::string& str);
    UByteCodeArg add_constant(const Value& value);

    void begin_scope();
    void end_scope();

    Token make_token(const TokenType type, std::string lexeme) const;

    std::unique_ptr<FunctionScope> release_enclosing();
    std::unique_ptr<CodeObject> release_code_object();
    std::vector<Upvalue> release_upvalues();

    void add_instruction(const Instruction instruction);
    template <typename T>
    void add_integer(const T integer);
    template <typename T>
    void rewrite_integer(const std::size_t pos, const T integer);
    void update_line_num_table(const Token& token);

    FunctionType type() const { return type_; }
    unsigned int scope_depth() const { return scope_depth_; }
    unsigned int last_line_num() const { return last_line_num_; }
    std::size_t num_upvalues() const { return upvalues_.size(); }
    std::size_t current_bytecode_size() const
    { return code_object_->bytecode.size(); }
    const CodeObject& code_object() const { return *code_object_; }

    struct Local
    {
      bool defined;
      bool is_upvalue;
      std::size_t depth;
      std::string name;
    };

    struct Upvalue
    {
      bool is_local;
      UByteCodeArg index;
    };

  private:
    FunctionType type_;
    unsigned int last_line_num_;
    std::size_t last_instr_num_;
    unsigned int scope_depth_;
    std::vector<Local> locals_;
    std::vector<Upvalue> upvalues_;
    std::unique_ptr<FunctionScope> enclosing_;
    std::unique_ptr<CodeObject> code_object_;
  };


  template<typename T>
  void FunctionScope::add_integer(const T integer)
  {
    auto& bytecode = code_object_->bytecode;
    const auto integer_ptr = reinterpret_cast<const std::uint8_t*>(&integer);
    bytecode.insert(bytecode.end(), integer_ptr, integer_ptr + sizeof(T));
  }


  template<typename T>
  void FunctionScope::rewrite_integer(const std::size_t pos, const T integer)
  {
    *reinterpret_cast<T*>(code_object_->bytecode.data() + pos) = integer;
  }
}

#endif //LOXX_FUNCTIONSCOPE_HPP
