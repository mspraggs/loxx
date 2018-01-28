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
  class ClassInstance;


  class FuncCallableImpl : public Callable
  {
  public:
    FuncCallableImpl(std::shared_ptr<Environment> closure,
                     const bool is_initialiser)
        : is_initialiser_(is_initialiser), closure_(std::move(closure))
    {}

    std::shared_ptr<FuncCallableImpl> bind(
        std::shared_ptr<ClassInstance> instance) const;

    Generic call(Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override;

    unsigned int arity() const override;

  protected:
    virtual const std::vector<Token>& params() const = 0;
    virtual const std::vector<std::shared_ptr<Stmt>>& body() const = 0;
    virtual std::shared_ptr<FuncCallableImpl> wrap_environment(
        std::shared_ptr<Environment> environment) const = 0;

    bool is_initialiser_;

  private:
    std::shared_ptr<Environment> closure_;
  };


  template <typename T>
  class FuncCallable : public FuncCallableImpl
  {
  public:
    FuncCallable(T declaration, std::shared_ptr<Environment> closure,
                 const bool is_initialiser)
        : FuncCallableImpl(std::move(closure), is_initialiser),
          declaration_(std::move(declaration))
    {}

  protected:
    const std::vector<Token>& params() const override
    { return declaration_.parameters; }

    const std::vector<std::shared_ptr<Stmt>>& body() const override
    { return declaration_.body; }

    std::shared_ptr<FuncCallableImpl> wrap_environment(
        std::shared_ptr<Environment> environment) const override
    {
      return std::make_shared<FuncCallable<T>>(declaration_, environment,
                                               is_initialiser_);
    }

  private:
    T declaration_;
  };
}

#endif //LOXX_FUNCCALLABLE_HPP
