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
 * Created by Matt Spraggs on 24/01/2018.
 */

#include "ClassInstance.hpp"
#include "RuntimeError.hpp"


namespace loxx
{
  Generic ClassInstance::get(const Token& name) const
  {
    if (fields_.count(name.lexeme()) != 0) {
      return fields_.at(name.lexeme());
    }

    const auto method = cls_->find_method(
        std::const_pointer_cast<ClassInstance>(this->shared_from_this()),
        name.lexeme());
    if (method.has_type<std::shared_ptr<Callable>>()) {
      return method;
    }

    const auto msg = std::string("Undefined property '") + name.lexeme() + "'.";
    throw RuntimeError(name, msg);
  }


  void ClassInstance::set(const Token& name, Generic value)
  {
    const bool have_field = fields_.count(name.lexeme()) != 0;

    if (have_field) {
      fields_.at(name.lexeme()) = std::move(value);
    }
    else {
      fields_.emplace(name.lexeme(), std::move(value));
    }
  }
}
