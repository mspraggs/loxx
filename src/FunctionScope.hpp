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
#include "Stmt.hpp"
#include "globals.hpp"
#include "Instruction.hpp"
#include "Optional.hpp"
#include "Token.hpp"


namespace loxx
{
  enum class FunctionType {
    FUNCTION,
    INITIALISER,
    METHOD,
    NONE
  };


  class FunctionScope
  {
  public:
    struct Upvalue;

    FunctionScope(
        const FunctionType type,
        const Function& func,
        std::unique_ptr<FunctionScope> enclosing = nullptr)
        : type_(type), last_line_num_(0), last_instr_num_(0),
          scope_depth_(enclosing == nullptr ? 0 : enclosing->scope_depth_ + 1),
          enclosing_(std::move(enclosing)), code_object_(new CodeObject)
    {
      code_object_->name = func.name.lexeme();
      insert_block_edge(0);

      if (type_ == FunctionType::FUNCTION) {
        add_local("");
        code_object_->varnames[0] = func.name.lexeme();
      }

      for (const auto& param : func.parameters) {
        code_object_->varnames.push_back(param.lexeme());
      }
      code_object_->num_args = static_cast<unsigned int>(
          func.parameters.size());
    }

    explicit FunctionScope(const FunctionType type)
        : FunctionScope(
              type,
              Function(Token(TokenType::IDENTIFIER, "top level", 0), {}, {}))
    {
    }

    void declare_local(const Token& name);
    void define_local();
    void add_local(const std::string& name);

    Optional<InstrArgUByte> resolve_local(
        const Token& name, const bool in_function) const;
    Optional <InstrArgUByte> resolve_upvalue(const Token& name);
    InstrArgUByte add_upvalue(const InstrArgUByte index, const bool is_local);

    InstrArgUByte add_named_constant(const std::string& lexeme,
                                    const Value& value);
    InstrArgUByte add_string_constant(const std::string& str);
    InstrArgUByte add_constant(const Value& value);
    void add_type_profile_instr(const Token& token, const bool write);

    void begin_scope();
    void end_scope();

    Token make_token(const TokenType type, std::string lexeme) const;

    std::unique_ptr<FunctionScope> release_enclosing();
    std::unique_ptr<CodeObject> release_code_object();
    std::vector<Upvalue> release_upvalues();

    void add_instruction(const Instruction instruction);
    template <typename T>
    void add_integer(const T integer);
    std::size_t add_jump(const Instruction instruction);
    void patch_jump(const std::size_t pos);
    void add_loop(const Instruction instruction, const std::size_t pos);
    template <typename T>
    void rewrite_integer(const std::size_t pos, const T integer);
    void update_line_num_table(const Token& token);
    void insert_block_edge(const std::size_t offset);

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
      InstrArgUByte index;
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
    StringHashSet type_profile_cache_;  // TODO: Better name...
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
