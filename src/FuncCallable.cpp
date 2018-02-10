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

#include <utility>

#include "ClassInstance.hpp"
#include "FuncCallable.hpp"
#include "Interpreter.hpp"


namespace loxx
{
  unsigned int FuncCallableImpl::arity() const
  {
    return static_cast<unsigned int>(params().size());
  }


  std::shared_ptr<FuncCallableImpl> FuncCallableImpl::bind(
      std::shared_ptr<ClassInstance> instance) const
  {
    auto environment = std::make_shared<Environment>(closure_);
    environment->define("this", std::move(instance));
    return wrap_environment(environment);
  }


  Generic FuncCallableImpl::call(Interpreter& interpreter,
                                 const std::vector<Generic>& arguments)
  {
    auto environment = std::make_shared<Environment>(closure_);

    for (unsigned int i = 0; i < params().size(); ++i) {
      environment->define(params()[i].lexeme(), arguments[i]);
    }

    try {
      interpreter.execute_block(body(), environment);
    }
    catch (const Interpreter::Returner& e) {
      return e.value();
    }

    if (is_initialiser_) {
      return closure_->get_at(0, "this");
    }

    return Generic(nullptr);
  }
}
