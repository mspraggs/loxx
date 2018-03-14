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
    Scope() : num_locals_(0) {}

    explicit Scope(std::unique_ptr<Scope> enclosing)
        : enclosing_(std::move(enclosing)), num_locals_(0)
    {}

    ByteCodeArg make_variable(const std::string& lexeme);

    std::tuple<ByteCodeArg, ByteCodeArg> resolve(
        const std::string& lexeme, const ByteCodeArg depth = 0) const;

    std::unique_ptr<Scope> release_enclosing();

    ByteCodeArg get_depth(const ByteCodeArg depth = 0) const;

    ByteCodeArg num_locals() const { num_locals_; }

  private:
    ByteCodeArg num_locals_;
    std::unique_ptr<Scope> enclosing_;
    std::unordered_map<std::string, ByteCodeArg> value_map_;
  };
}

#endif //LOXX_SCOPE_HPP
