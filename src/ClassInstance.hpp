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
 * Created by Matt Spraggs on 24/01/2018.
 */

#ifndef LOXX_CLASSINSTANCE_HPP
#define LOXX_CLASSINSTANCE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ClassDef.hpp"
#include "Generic.hpp"
#include "Token.hpp"


namespace loxx
{
  class ClassInstance : public std::enable_shared_from_this<ClassInstance>
  {
  public:
    explicit ClassInstance(std::shared_ptr<ClassDef> cls)
        : cls_(std::move(cls))
    {}

    const ClassDef& cls() const { return *cls_; }

    Generic get(const Token& name) const;
    void set(const Token& name, Generic value);
    
  private:
    std::shared_ptr<ClassDef> cls_;
    std::unordered_map<std::string, Generic> fields_;
  };
}

#endif // LOXX_CLASSINSTANCE_HPP
