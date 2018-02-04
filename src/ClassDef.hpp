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

#ifndef LOXX_CLASSDEF_HPP
#define LOXX_CLASSDEF_HPP

#include <string>
#include <unordered_map>

#include "Callable.hpp"
#include "FuncCallable.hpp"


namespace loxx
{
  class ClassInstance;

  
  class ClassDef : public Callable
  {
  public:
    ClassDef(std::string name,
	      std::unordered_map<std::string, Generic> methods)
        : name_(std::move(name)), methods_(std::move(methods))
    {
    }

    Generic find_method(std::shared_ptr<ClassInstance> instance,
                        const std::string& name) const;

    unsigned int arity() const override;
    Generic call(Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override;

    const std::string& name() const { return name_; }
    
  private:
    std::string name_;
    std::unordered_map<std::string, Generic> methods_;
  };
}

#endif // LOXX_CLASSDEF_HPP
