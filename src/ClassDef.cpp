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

#include "ClassDef.hpp"
#include "ClassInstance.hpp"


namespace loxx
{
  Generic ClassDef::find_method(std::shared_ptr<ClassInstance> instance,
                                const std::string& name) const
  {
    if (bound_methods_.count(name) != 0) {
      const auto& method =
          bound_methods_.at(name).get<Callable, FuncCallable<Function>>();
      std::shared_ptr<Callable> bound_method = method.bind(std::move(instance));
      return Generic(bound_method);
    }

    if (static_methods_.count(name) != 0) {
      return static_methods_.at(name);
    }

    return Generic(nullptr);
  }


  unsigned int ClassDef::arity() const
  {
    if (bound_methods_.count("init") == 0) {
      return 0;
    }
    return bound_methods_.at("init").get<Callable>().arity();
  }

  
  Generic ClassDef::call(Interpreter& interpreter,
                         const std::vector<Generic>& arguments)
  {
    const auto instance = this->shared_from_this();

    if (bound_methods_.count("init") != 0) {
      auto initialiser =
          bound_methods_.at("init").get<Callable, FuncCallable<Function>>()
              .bind(instance);
      initialiser->call(interpreter, arguments);
    }

    return Generic(instance);
  }
}
