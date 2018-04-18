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
 * Created by Matt Spraggs on 18/04/18.
 */

#include "Value.hpp"


namespace loxx
{
  bool ClassObject::has_method(const std::string& name) const
  {
    bool ret = methods_.count(name) != 0;

    if (base_) {
      ret |= base_->has_method(name);
    }

    return ret;
  }


  std::shared_ptr<ClosureObject> ClassObject::method(
      const std::string& name) const
  {
    if (methods_.count(name) != 0) {
      return methods_.at(name);
    }
    if (base_) {
      return base_->method(name);
    }
    return std::shared_ptr<ClosureObject>();
  }


  std::shared_ptr<ClosureObject> ClassObject::method(const std::string& name)
  {
    if (methods_.count(name) != 0) {
      return methods_[name];
    }
    if (base_) {
      return base_->method(name);
    }
    return std::shared_ptr<ClosureObject>();
  }
}