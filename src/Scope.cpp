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


namespace loxx
{
  void Scope::declare_variable(const std::string& lexeme)
  {
    if (enclosing_ == nullptr) {
      // Global variables are implicitly declared
      return;
    }

    if (value_map_.count(lexeme) != 0) {
      throw std::logic_error("TODO: Fix this(?)");
    }

    const auto index = value_defined_.size();
    value_map_[lexeme] = index;
    value_defined_.push_back(false);
  }


  void Scope::define_variable(const ByteCodeArg arg)
  {
    if (enclosing_ == nullptr) {
      // Global variables are implicitly declared
      return;
    }

    value_defined_[arg] = true;
  }


  ByteCodeArg Scope::make_variable(const std::string& lexeme)
  {
    if (value_map_.count(lexeme) != 0) {
      return value_map_[lexeme];
    }

    value_map_[lexeme] = scope_depth_;

    return scope_depth_++;
  }


  std::tuple<bool, ByteCodeArg, ByteCodeArg> Scope::resolve(
      const std::string& name) const
  {
    if (value_map_.count(name) != 0) {
      return std::make_tuple(true, value_map_.at(name), scope_depth_);
    }

    if (enclosing_ != nullptr) {
      return enclosing_->resolve(name);
    }

    return std::make_tuple(false, 0, 0);
  }


  std::unique_ptr<Scope> Scope::release_enclosing()
  {
    std::unique_ptr<Scope> ret;
    std::swap(enclosing_, ret);
    return ret;
  }
}
