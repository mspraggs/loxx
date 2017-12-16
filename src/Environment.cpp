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
    const std::size_t index = first_value ? values_.size() : value_map_[name];

    if (first_value) {
      values_.push_back(std::move(value));
      is_defined_.push_back(true);
      value_map_[name] = index;
    }
    else {
      values_[index] = std::move(value);
      is_defined_[index] = true;
    }
  }


  void Environment::define(const std::size_t index, Generic value)
  {
    if (index >= values_.size()) {
      values_.resize(index + 1, Generic(nullptr));
      is_defined_.resize(index + 1, false);
    }
    values_[index] = std::move(value);
    is_defined_[index] = true;
  }


  const Generic& Environment::get(const Token& name) const
  {
    if (value_map_.count(name.lexeme()) != 0) {
      return values_[value_map_.at(name.lexeme())];
    }

    if (enclosing_ != nullptr) {
      return enclosing_->get(name);
    }


    throw RuntimeError(name, "Undefined variable '" + name.lexeme() + "'.");
  }


  const Generic& Environment::get(const std::size_t index) const
  {
    if (is_defined_.at(index)) {
      return values_[index];
    }
    throw std::out_of_range("Variable undefined!");
  }


  const Generic& Environment::get_at(const std::size_t distance,
                                     const std::string& name) const
  {
    const Environment& env = ancestor(distance);
    return env.values_[env.value_map_.at(name)];
  }


  const Generic& Environment::get_at(const std::size_t distance,
                                     const std::size_t index) const
  {
    const Environment& env = ancestor(distance);
    return env.values_.at(index);
  }


  void Environment::assign(const Token& name, Generic value)
  {
    if (value_map_.count(name.lexeme())) {
      values_[value_map_[name.lexeme()]] = std::move(value);
      return;
    }

    if (enclosing_ != nullptr) {
      enclosing_->assign(name, std::move(value));
      return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme() + "'.");
  }


  void Environment::assign_at(const std::size_t distance, const Token& name,
                              Generic value)
  {
    Environment& env = ancestor(distance);
    env.values_[env.value_map_.at(name.lexeme())] = std::move(value);
  }


  void Environment::assign_at(const std::size_t distance,
                              const std::size_t index, Generic value)
  {
    Environment& env = ancestor(distance);
    env.values_[index] = std::move(value);
  }


  std::shared_ptr<Environment> Environment::release_enclosing()
  {
    std::shared_ptr<Environment> ret;
    std::swap(enclosing_, ret);
    return ret;
  }


  const Environment& Environment::ancestor(const std::size_t distance) const
  {
    const Environment* environment = this;
    for (std::size_t i = 0; i < distance; ++i) {
      environment = environment->enclosing_.get();
    }

    return *environment;
  }


  Environment& Environment::ancestor(const std::size_t distance)
  {
    Environment* environment = this;
    for (std::size_t i = 0; i < distance; ++i) {
      environment = environment->enclosing_.get();
    }

    return *environment;
  }
}