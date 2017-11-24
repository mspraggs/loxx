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
 * Created by Matt Spraggs on 24/11/17.
 */

#ifndef LOXX_NATIVECALLABLE_HPP
#define LOXX_NATIVECALLABLE_HPP

#include "Callable.hpp"


namespace loxx
{
  template <typename Fn>
  class NativeCallable : public Callable
  {
  public:
    NativeCallable(Fn func, unsigned int arity)
        : func_(std::move(func)), arity_(arity)
    {}

    unsigned int arity() const override { return arity_; }

    Generic call(const Interpreter& interpreter,
                 const std::vector<Generic>& arguments) override
    {
      return func_(interpreter, arguments);
    }

  private:
    Fn func_;
    unsigned int arity_;
  };
}

#endif //LOXX_NATIVECALLABLE_HPP
