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
 * Created by Matt Spraggs on 16/11/17.
 */

#include "Environment.hpp"
#include "RuntimeError.hpp"


namespace loxx
{
  void Environment::define(std::string name, Generic value)
  {
    const bool first_value = value_map_.count(name) == 0;
    const std::size_t index = first_value ? value_map_[name] : values_.size();

    if (first_value) {
      values_.emplace_back(std::move(value));
      value_map_[name] = index;
    }
    else {
      values_[index] = std::move(value);
    }
  }


  const Generic& Environment::get(const Token& name) const
  {
    if (value_map_.count(name.lexeme()) != 0) {
      return values_[value_map_.at(name.lexeme())];
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme() + "'.");
  }
}