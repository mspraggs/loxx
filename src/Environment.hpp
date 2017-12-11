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

#ifndef LOXX_ENVIRONMENT_HPP
#define LOXX_ENVIRONMENT_HPP

#include <unordered_map>
#include <vector>

#include "Generic.hpp"
#include "Token.hpp"


namespace loxx
{
  class Environment
  {
  public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> enclosing)
        : enclosing_(std::move(enclosing))
    {}

    void define(std::string name, Generic value);
    const Generic& get(const Token& name) const;
    void assign(const Token& name, Generic value);

    std::shared_ptr<Environment> release_enclosing();

  private:
    const Environment& ancestor(const std::size_t distance) const;
    Environment& ancestor(const std::size_t distance);

    std::shared_ptr<Environment> enclosing_;
    std::unordered_map<std::string, std::size_t> value_map_;
    std::vector<Generic> values_;
  };
}

#endif //LOXX_ENVIRONMENT_HPP
