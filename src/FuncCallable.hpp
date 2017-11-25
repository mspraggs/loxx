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

#ifndef LOXX_FUNCTIONOBJ_HPP
#define LOXX_FUNCTIONOBJ_HPP

#include "Callable.hpp"
#include "Stmt.hpp"


namespace loxx
{
  class FuncCallable : public Callable
  {
  public:
    FuncCallable(const Function& declaration)
        : declaration_(&declaration)
    {}

    Generic call(Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override;

    unsigned int arity() const override;

  private:
    const Function* declaration_;
  };
}

#endif //LOXX_FUNCTIONOBJ_HPP
