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
 * Created by Matt Spraggs on 06/11/2019.
 */

#ifndef LOXX_JIT_IRINSTRUCTION_HPP
#define LOXX_JIT_IRINSTRUCTION_HPP

#include <array>
#include <iomanip>

#include "../HashTable.hpp"
#include "../Object.hpp"
#include "../Value.hpp"


namespace loxx
{
  namespace jit
  {
    enum class Operator
    {
      ADD,
      CHECK_FALSE,
      CHECK_TRUE,
      CHECK_TYPE,
      DIVIDE,
      EQUAL,
      JUMP,
      LESS,
      LITERAL,
      LOAD,
      LOOP,
      LOOP_START,
      MOVE,
      MULTIPLY,
      NOOP,
      PHI,
      POP,
      RETURN,
      STORE,
      SUBTRACT,
    };


    struct Operand
    {
    public:
      enum class Type
      {
        IR_REF,
        STACK_REF,
        JUMP_OFFSET,
        EXIT_NUMBER,
        LITERAL_BOOLEAN,
        LITERAL_FLOAT,
        LITERAL_OBJECT,
        LITERAL_NIL,
        UNUSED,
      };

      Operand();
      Operand(const Type type, const std::size_t value);
      Operand(const Type type);
      explicit Operand(const double value);
      explicit Operand(const bool value);
      explicit Operand(const ObjectPtr value);

      Type type() const { return type_; }
      bool is_literal() const;

    private:
      template <typename T>
      friend T unsafe_get(const Operand& operand);

      template <typename T>
      friend T get(const Operand& operand);

      friend bool operator==(
          const Operand& first, const Operand& second);

      Type type_;
      std::aligned_storage_t<8, 8> storage_;
    };


    class BadOperandAccess : public std::exception
    {
    public:
      explicit BadOperandAccess(std::string what)
          : std::exception(), what_(std::move(what))
      {}
      const char* what() const noexcept override { return what_.c_str(); }

    private:
      std::string what_;
    };


    template <typename T>
    T unsafe_get(const Operand& operand);


    template <typename T>
    T get(const Operand& operand);


    template <std::size_t N>
    class IRInstruction
    {
    public:
      template <typename... Args>
      explicit IRInstruction(
          const Operator op, const ValueType type, const Args&... operands);

      Operator op() const { return op_; }
      ValueType type() const { return type_; }
      const std::array<Operand, N>& operands() const { return operands_; }
      const Operand& operand(const std::size_t i) const { return operands_[i]; }
      void set_operand(const std::size_t i, const Operand& value)
      { operands_[i] = value; }

      bool is_loop_invariant() const;

    private:
      ValueType type_;
      Operator op_;
      std::array<Operand, N> operands_;
    };


    template <std::size_t N>
    using IRBuffer = std::vector<IRInstruction<N>>;


    template <std::size_t N>
    template <typename... Args>
    IRInstruction<N>::IRInstruction(
        const Operator op, const ValueType type, const Args&... operands)
        : type_(type), op_(op), operands_{operands...}
    {
    }


    template <std::size_t N>
    bool IRInstruction<N>::is_loop_invariant() const
    {
      if (op_ == Operator::STORE or
          op_ == Operator::LOAD or
          op_ == Operator::CHECK_TYPE) {
        return true;
      }
      return false;
    }


    bool operator==(const Operand& first, const Operand& second);


    template <typename Os>
    Os& operator<<(Os& os, const Operand::Type type)
    {
      switch (type)
      {
      case Operand::Type::IR_REF:
        os << "IR";
        break;
      case Operand::Type::STACK_REF:
        os << "STACK";
        break;
      case Operand::Type::JUMP_OFFSET:
        os << "JMP";
        break;
      case Operand::Type::EXIT_NUMBER:
        os << "EXIT";
        break;
      case Operand::Type::LITERAL_BOOLEAN:
        os << "BOOL";
        break;
      case Operand::Type::LITERAL_FLOAT:
        os << "FLOAT";
        break;
      case Operand::Type::LITERAL_OBJECT:
        os << "OBJ";
        break;
      case Operand::Type::LITERAL_NIL:
        os << "NIL";
        break;
      case Operand::Type::UNUSED:
        os << "---";
        break;
      }

      return os;
    }


    template <typename Os>
    Os& operator<<(Os& os, const Operator op)
    {
      switch (op) {
      case Operator::ADD:
        os << "ADD";
        break;
      case Operator::CHECK_FALSE:
        os << "CHECK_FALSE";
        break;
      case Operator::CHECK_TRUE:
        os << "CHECK_TRUE";
        break;
      case Operator::CHECK_TYPE:
        os << "CHECK_TYPE";
        break;
      case Operator::DIVIDE:
        os << "DIVIDE";
        break;
      case Operator::EQUAL:
        os << "EQUAL";
        break;
      case Operator::JUMP:
        os << "JUMP";
        break;
      case Operator::LESS:
        os << "LESS";
        break;
      case Operator::LITERAL:
        os << "LITERAL";
        break;
      case Operator::LOAD:
        os << "LOAD";
        break;
      case Operator::LOOP:
        os << "LOOP";
        break;
      case Operator::LOOP_START:
        os << "LOOP_START";
        break;
      case Operator::MOVE:
        os << "MOVE";
        break;
      case Operator::MULTIPLY:
        os << "MULTIPLY";
        break;
      case Operator::NOOP:
        os << "NOOP";
        break;
      case Operator::PHI:
        os << "PHI";
        break;
      case Operator::POP:
        os << "POP";
        break;
      case Operator::RETURN:
        os << "RETURN";
        break;
      case Operator::STORE:
        os << "STORE";
        break;
      case Operator::SUBTRACT:
        os << "SUBTRACT";
        break;
      }

      return os;
    }


    template <typename Os>
    Os& operator<<(Os& os, const Operand& operand)
    {

      os << std::setw(5) << std::setfill('.') << std::left;
      os << operand.type() << "[ ";
      if (not operand.is_literal()) {
        os << std::setw(4) << std::setfill('0') << std::right;
        os << unsafe_get<std::size_t>(operand);
      }
      else if (operand.type() == Operand::Type::LITERAL_FLOAT) {
        os << '\'' <<  unsafe_get<double>(operand) << '\'';
      }
      else if (operand.type() == Operand::Type::LITERAL_BOOLEAN) {
        os << '\'' <<  unsafe_get<bool>(operand) << '\'';
      }
      else if (operand.type() == Operand::Type::LITERAL_OBJECT) {
        os << '\'' <<  unsafe_get<ObjectPtr>(operand) << '\'';
      }
      os << " ]";

      return os;
    }
  }
}

#endif // LOXX_JIT_IRINSTRUCTION_HPP
