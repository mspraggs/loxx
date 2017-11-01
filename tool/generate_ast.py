"""
This file is part of loxx.

loxx is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

loxx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Created by Matt Spraggs on 01/11/17.
"""

import os
import sys

import jinja2


def define_ast(output_dir, base_name, types):
    """Generate AST class definitions using specification and jinja2"""

    class_specs = []

    for name, members in types.items():
        arglist = ", ".join(" ".join(p) for p in members)
        initialisers = ", ".join("{}_(std::move({}))".format(n, n)
                                 for t, n in members)
        member_vars = "\n    ".join("{} {}_;".format(*p) for p in members)
        accessors = "\n    ".join("const {0}& {1}() const {{ return {1}_; }}"
                                  .format(*p) for p in members)

        class_specs.append(dict(name=name, arglist=arglist,
                                initialisers=initialisers, members=member_vars,
                                accessors=accessors))

    with open("Expressions.hpp.template") as f:
        template = jinja2.Template(f.read())

    with open(os.path.join(output_dir, "Expressions.hpp"), 'w') as f:
        f.write(template.render(base_name=base_name, class_specs=class_specs))


if __name__ == "__main__":

    try:
        output_dir = sys.argv[1]
    except IndexError:
        print("Usage: python {} <output directory>".format(sys.argv[0]))
        sys.exit(1)

    define_ast(
        output_dir, "Expr",
        {"Binary": [("Expr", "left"), ("Token", "op"), ("Expr", "right")],
         "Grouping": [("Expr", "expression")],
         "NumberLiteral": [("double", "value")],
         "StringLiteral": [("std::string", "value")],
         "Unary": [("Token", "op"), ("Expr", "right")]})