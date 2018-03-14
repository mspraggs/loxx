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
 * Created by Matt Spraggs on 14/03/2018.
 */

#include "Scope.hpp"
#include "RuntimeError.hpp"


namespace loxx
{
  ByteCodeArg Scope::make_variable(const std::string& lexeme)
  {
    if (value_map_.count(name) != 0) {
      return value_map_[lexeme];
    }

    ByteCodeArg index = values_.size();
    value_map_[lexeme] = index;
  }
  
  std::tuple<ByteCodeArg, ByteCodeArg> Scope::resolve(
      const std::string& name, const ByteCodeArg depth) const
  {
    if (value_map_.count(name) != 0) {
      return std::make_tuple(value_map_.at(name), depth);
    }

    if (enclosing_ != nullptr) {
      return enclosing_->resolve(name, depth + 1);
    }

    // TODO: Error handling
  }
}
