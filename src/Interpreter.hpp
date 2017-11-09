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
 * Created by Matt Spraggs on 05/11/17.
 */

#ifndef LOXX_INTERPRETER_HPP
#define LOXX_INTERPRETER_HPP

#include <stack>
#include <vector>

#include "Expressions.hpp"


namespace loxx
{
  class Interpreter : public Visitor
  {
  public:
    void interpret(const Expr& expr);

    void visitUnaryExpr(const Unary& expr) override;
    void visitBinaryExpr(const Binary& expr) override;
    void visitLiteralExpr(const Literal& expr) override;
    void visitTernaryExpr(const Ternary& expr) override;
    void visitGroupingExpr(const Grouping& expr) override;

    class RuntimeError : public std::runtime_error
    {
    public:
      RuntimeError(Token token, const std::string& message)
          : std::runtime_error(message), token_(std::move(token))
      {}

      const Token& token() const { return token_; }

    private:
      Token token_;
    };

  private:
    void evaluate(const Expr& expr);
    bool is_truthy(const Generic& value);
    bool is_equal(const Generic& left, const Generic& right);
    void check_number_operand(const Token& op, const Generic& value) const;
    void check_number_operands(const Token& op,
                               const Generic& left, const Generic& right) const;
    std::string stringify(Generic& generic) const;

    std::stack<Generic, std::vector<Generic>> stack_;
  };
}

#endif //LOXX_INTERPRETER_HPP
