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
 * Created by Matt Spraggs on 01/11/17.
 */

#ifndef LOXX_{{ base_name|upper }}_HPP
#define LOXX_{{ base_name|upper }}_HPP

#include <memory>
#include <vector>

#include "Generic.hpp"
#include "Token.hpp"
{% for inc in includes %}#include "{{ inc }}"
{% endfor %}

namespace loxx
{
{% for decl in forward_decls %}
  {{ decl }};{% endfor %}
{% for spec in class_specs %}
  struct {{ spec.name }};{% endfor %}

  struct {{ base_name }}
  {
    virtual ~{{ base_name }}() = default;

    class Visitor
    {
    public:{% for spec in class_specs %}
      virtual void visit_{{ spec.name|lower }}_{{ base_name|lower }}(const {{ spec.name }}& {{ base_name|lower }}) = 0;{% endfor %}
    };

    virtual void accept(Visitor&) const {}
  };
{% for spec in class_specs %}

  struct {{ spec.name }} : public {{ base_name }}
  {
    {{ spec.name }}({{ spec.arglist }}){% if spec.initialisers %}
        : {{ spec.initialisers }}{% endif %}
    {}

    void accept(Visitor& visitor) const override
    { visitor.visit_{{ spec.name|lower }}_{{ base_name|lower }}(*this); }

    {{ spec.members }}
  };
{% endfor %}
}

#endif //LOXX_{{ base_name|upper }}_HPP
