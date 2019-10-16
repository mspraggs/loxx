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

#include <functional>
#include <iostream>

#include "CodeProfiler.hpp"


namespace loxx
{
  namespace detail
  {
    std::size_t combine_hashes(const std::size_t h0, const std::size_t h1)
    {
      return h0 ^ (0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }

    std::size_t compute_variable_type_hash(
        const CodeObject* code, const StringObject* varname,
        const std::size_t type)
    {
      const auto varname_hash = varname->hash();
      const auto code_hash = std::hash<const CodeObject*>()(code);

      return combine_hashes(combine_hashes(varname_hash, code_hash), type);
    }
  }

  void CodeProfiler::count_variable_type(
      const CodeObject* code, const StringObject* varname,
      const std::size_t type)
  {
    const auto hash = detail::compute_variable_type_hash(code, varname, type);
    const auto type_info = VariableTypeInfo{code, varname, type, 0};
    auto& count_elem = variable_type_counts_.insert(hash, type_info);
    count_elem->second.count += 1;
  }


  void CodeProfiler::count_function_call(
      const CodeObject* code, const ClosureObject* function,
      const std::size_t num_args, const Value* args)
  {
    auto hash = std::hash<const CodeObject*>()(code);
    const auto function_hash = std::hash<const ClosureObject*>()(function);
    hash = detail::combine_hashes(hash, function_hash);

    std::vector<std::size_t> types(num_args);
    for (std::size_t i = 0; i < num_args; ++i) {
      types[i] = args[i].index();
      hash = detail::combine_hashes(hash, types[i]);
    }

    const auto call_info = FunctionCallInfo{code, function, types, 0};
    auto& count_elem = function_call_counts_.insert(hash, call_info);
    count_elem->second.count += 1;
  }
}
