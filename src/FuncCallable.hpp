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


  class FuncCallable : public Callable
  {
  public:
    FuncCallable(Function declaration, std::shared_ptr<Environment> closure,
                 const bool is_initialiser)
        : is_initialiser_(is_initialiser), declaration_(std::move(declaration)),
          closure_(std::move(closure))
    {}

    std::shared_ptr<FuncCallable> bind(
        std::shared_ptr<ClassInstance> instance) const;

    Generic call(Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override;

    unsigned int arity() const override;

  private:
    bool is_initialiser_;
    Function declaration_;
    std::shared_ptr<Environment> closure_;
  };
}

#endif //LOXX_FUNCCALLABLE_HPP
