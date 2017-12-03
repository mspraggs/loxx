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
 * Created by Matt Spraggs on 25/11/17.
 */

#ifndef LOXX_FUNCCALLABLE_HPP
#define LOXX_FUNCCALLABLE_HPP

#include "Callable.hpp"

#include "Environment.hpp"
#include "Stmt.hpp"


namespace loxx
{
  class FuncCallableImpl : public Callable
  {
  public:
    FuncCallableImpl(std::shared_ptr<Environment> closure)
        : closure_(std::move(closure))
    {}

    Generic call(Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override;

    unsigned int arity() const override;

    virtual const std::vector<Token>& params() const = 0;
    virtual const std::vector<std::shared_ptr<Stmt>>& body() const = 0;

  private:
    std::shared_ptr<Environment> closure_;
  };


  template <typename T>
  class FuncCallable : public FuncCallableImpl
  {
  public:
    FuncCallable(T declaration, std::shared_ptr<Environment> closure)
        : FuncCallableImpl(std::move(closure)),
          declaration_(std::move(declaration))
    {}

    const std::vector<Token>& params() const override
    { return declaration_.parameters(); }

    const std::vector<std::shared_ptr<Stmt>>& body() const override
    { return declaration_.body(); }

  private:
    T declaration_;
  };
}

#endif //LOXX_FUNCCALLABLE_HPP
