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
 * Created by Matt Spraggs on 23/11/17.
 */

#ifndef LOXX_CALLABLE_HPP
#define LOXX_CALLABLE_HPP

#include <vector>

#include "Generic.hpp"


namespace loxx
{
  class Interpreter;

  class Callable : public std::enable_shared_from_this<Callable>
  {
  public:
    virtual unsigned int arity() const = 0;

    virtual Generic call(Interpreter& interpreter,
                         const std::vector<Generic>& arguments) = 0;

    virtual const std::string& name() const = 0;
  };
}

#endif //LOXX_CALLABLE_HPP
