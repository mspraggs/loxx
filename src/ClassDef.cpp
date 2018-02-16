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
    if (methods_.count(name) != 0) {
      const auto method = std::static_pointer_cast<FuncCallable>(
          generic_cast<std::shared_ptr<Callable>>(methods_.at(name)));
      std::shared_ptr<Callable> bound_method = method->bind(std::move(instance));
      return Generic(bound_method);
    }

    if (superclass_ != nullptr) {
      return superclass_->find_method(std::move(instance), name);
    }

    return Generic(nullptr);
  }


  unsigned int ClassDef::arity() const
  {
    if (methods_.count("init") == 0) {
      return 0;
    }
    return generic_cast<std::shared_ptr<Callable>>(methods_.at("init"))->arity();
  }

  
  Generic ClassDef::call(Interpreter& interpreter,
                         const std::vector<Generic>& arguments)
  {
    const auto cls =
        std::static_pointer_cast<ClassDef>(this->shared_from_this());
    const auto instance = std::make_shared<ClassInstance>(cls);

    if (methods_.count("init") != 0) {
      auto initialiser = std::static_pointer_cast<FuncCallable>(
          generic_cast<std::shared_ptr<Callable>>(methods_.at("init")))
          ->bind(instance);
      initialiser->call(interpreter, arguments);
    }

    return Generic(instance);
  }
}
