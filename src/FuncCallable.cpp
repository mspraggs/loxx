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

#include "Environment.hpp"
#include "FuncCallable.hpp"
#include "Interpreter.hpp"


namespace loxx
{
  unsigned int FuncCallable::arity() const
  {
    return static_cast<unsigned int>(declaration_->parameters().size());
  }


  Generic FuncCallable::call(Interpreter& interpreter,
                            const std::vector<Generic>& arguments)
  {
    auto environment =
        std::make_shared<Environment>(interpreter.get_global_env());

    for (unsigned int i = 0; i < declaration_->parameters().size(); ++i) {
      environment->define(declaration_->parameters()[i].lexeme(), arguments[i]);
    }

    interpreter.execute_block(declaration_->body(), environment);

    return Generic(nullptr);
  }
}