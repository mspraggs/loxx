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

#ifndef LOXX_SCOPE_HPP
#define LOXX_SCOPE_HPP

#include <unordered_map>
#include <vector>

#include "globals.hpp"
#include "Token.hpp"


namespace loxx
{
  class Scope
  {
  public:
    Scope() : scope_depth_(0) {}

    explicit Scope(std::unique_ptr<Scope> enclosing)
        : enclosing_(std::move(enclosing)),
          scope_depth_(enclosing_->scope_depth_ + 1)
    {}

    void declare_variable(const std::string& lexeme);

    void define_variable(const ByteCodeArg arg);

    ByteCodeArg make_variable(const std::string& lexeme);

    std::tuple<bool, ByteCodeArg, ByteCodeArg> resolve(
        const std::string& lexeme) const;

    std::unique_ptr<Scope> release_enclosing();

    ByteCodeArg get_depth() const { return scope_depth_; }

    ByteCodeArg num_locals() const { return value_defined_.size(); }

  private:
    ByteCodeArg scope_depth_;
    std::unique_ptr<Scope> enclosing_;
    std::unordered_map<std::string, ByteCodeArg> value_map_;
    std::vector<bool> value_defined_;
  };
}

#endif //LOXX_SCOPE_HPP
