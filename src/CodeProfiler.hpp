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
 * Created by Matt Spraggs on 13/10/2019.
 */

#ifndef LOXX_CODEPROFILER_HPP
#define LOXX_CODEPROFILER_HPP

#include <tuple>

#include "CodeObject.hpp"
#include "Object.hpp"


namespace loxx
{
  class CodeProfiler
  {
  public:
    void count_variable_type(
        const CodeObject* code, const StringObject* varname,
        const std::size_t type);

    void count_function_call(
        const CodeObject* code, const ClosureObject* function,
        const std::size_t num_args, const Value* args);

  private:
    struct VariableTypeInfo
    {
      const CodeObject* code;
      const StringObject* varname;
      std::size_t type;
      std::size_t count;
    };

    struct FunctionCallInfo
    {
      const CodeObject* code = nullptr;
      const ClosureObject* function = nullptr;
      std::vector<std::size_t> types = {};
      std::size_t count = 0;
    };

    HashTable<std::size_t, VariableTypeInfo> variable_type_counts_;
    HashTable<std::size_t, FunctionCallInfo> function_call_counts_;
  };
}

#endif // LOXX_CODEPROFILER_HPP
