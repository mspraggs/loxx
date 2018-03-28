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
 * Created by Matt Spraggs on 28/03/2018.
 */

#ifndef LOXX_VALUE_HPP
#define LOXX_VALUE_HPP

#include "Variant.hpp"

namespace loxx
{
  class Object
  {
  public:
    virtual ~Object() = default;

  protected:
    Object(const bool callable, const bool has_attributes,
           const std::size_t bytecode_offset)
        : callable_(callable), has_attributes_(has_attributes),
          bytecode_offset_(bytecode_offset)
    {}

    bool callable() const { return callable_; }
    bool has_attributes() const { return has_attributes_; }
    std::size_t bytecode_offset() const { return bytecode_offset_; }

  private:
    bool callable_, has_attributes_;
    std::size_t bytecode_offset_;
  };


  class FuncObject : public Object
  {
  public:
    FuncObject(const std::size_t bytecode_offset, const unsigned int arity)
        : Object(true, false, bytecode_offset), arity_(arity)
    {}

    unsigned int arity() const { return arity_; }

  private:
    unsigned int arity_;
  };

  
  using Value = Variant<double, bool, std::string, std::shared_ptr<Object>>;
}

#endif // LOXX_VALUE_HPP
