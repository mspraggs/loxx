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
 * Created by Matt Spraggs on 18/02/2020.
 */

#include "JITError.hpp"
#include "TraceCache.hpp"


namespace loxx
{
  namespace jit
  {
    Trace* TraceCache::get_trace(const CodeObject::InsPtr ip)
    {
      auto& result = trace_map_.get(ip);
      if (result) {
        return result->second;
      }
      return nullptr;
    }


    const Trace* TraceCache::get_trace(const CodeObject::InsPtr ip) const
    {
      const auto& result = trace_map_.get(ip);
      if (result) {
        return result->second;
      }
      return nullptr;
    }


    void TraceCache::make_new_trace()
    {
      active_.reset(new Trace);
      active_->state = Trace::State::NEW;
      active_->snaps.reserve(64);
    }


    void TraceCache::store_active_trace()
    {
      if (not active_) {
        throw JITError("no active trace exists");
      }

      const auto result = trace_map_.insert(active_->init_ip, active_.get());

      if (not result.second) {

        auto existing = result.first->second;
        while (true) {
          if (not existing->chained_trace) {
            existing->chained_trace = active_.get();
            break;
          }
          existing = existing->chained_trace;
        }

      }

      traces_.push_back(std::move(active_));
    }
  }
}
