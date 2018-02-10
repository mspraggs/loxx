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
 * Created by Matt Spraggs on 24/11/17.
 */

#include "Generic.hpp"


namespace loxx
{
  Generic::Generic(const Generic& generic)
      : container_(generic.container_->clone())
  {
  }


  Generic::Generic(Generic&& generic) noexcept
      : container_(std::move(generic.container_))
  {
  }


  Generic& Generic::operator=(const Generic& generic)
  {
    if (this != &generic) {
      container_.reset(generic.container_->clone());
    }

    return *this;
  }


  Generic& Generic::operator=(Generic&& generic) noexcept
  {
    container_ = std::move(generic.container_);

    return *this;
  }


  bool Generic::operator==(const Generic& generic) const
  {
    return *container_ == *generic.container_;
  }
}