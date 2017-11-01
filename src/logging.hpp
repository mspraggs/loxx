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
 * Created by Matt Spraggs on 31/10/17.
 */

#ifndef LOXX_LOGGING_HPP
#define LOXX_LOGGING_HPP

#include <string>


namespace loxx
{
  static bool had_error = false;


  void error(const unsigned int line, const std::string& message);


  void report(const unsigned int line, const std::string& where,
              const std::string& message);
}

#endif //LOXX_LOGGING_HPP
