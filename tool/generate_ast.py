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


def define_ast(output_dir, base_name, types, includes=[]):
    """Generate AST class definitions using specification and jinja2"""

    class_specs = []

    for name, members in types.items():
        arglist = ", ".join(
            "std::shared_ptr<{}> {}_arg".format(t, n)
            if i else "{} {}_arg".format(t, n)
            for t, n, i in members)
        initialisers = ", ".join(
            "{}(std::move({}_arg))".format(n, n)
            for t, n, i in members)
        member_vars = "\n    ".join(
            "std::shared_ptr<{}> {};".format(t, n)
            if i else "{} {};".format(t, n)
            for t, n, i in members)

        class_specs.append(dict(name=name, arglist=arglist,
                                initialisers=initialisers, members=member_vars))

    with open("ast_template.hpp") as f:
        template = jinja2.Template(f.read())

    with open(os.path.join(output_dir, "{}.hpp".format(base_name)), 'w') as f:
        f.write(template.render(base_name=base_name, class_specs=class_specs,
                                includes=includes))


if __name__ == "__main__":

    try:
        output_dir = sys.argv[1]
    except IndexError:
        print("Usage: python {} <output directory>".format(sys.argv[0]))
        sys.exit(1)

    define_ast(
        output_dir, "Expr",
        {"Assign": [("Token", "name", False), ("Expr", "value", True)],
         "Binary": [("Expr", "left", True), ("Token", "op", False),
                    ("Expr", "right", True)],
         "Call": [("Expr", "callee", True), ("Token", "paren", False),
                  ("std::vector<std::shared_ptr<Expr>>", "arguments", False)],
         "Get": [("Expr", "object", True), ("Token", "name", False)],
         "Grouping": [("Expr", "expression", True)],
         "Literal": [("Generic", "value", False)],
         "Logical": [("Expr", "left", True), ("Token", "op", False),
                     ("Expr", "right", True)],
         "Set": [("Expr", "object", True), ("Token", "name", False),
                 ("Expr", "value", True)],
         "Super": [("Token", "keyword", False), ("Token", "method", False)],
         "This": [("Token", "keyword", False)],
         "Unary": [("Token", "op", False), ("Expr", "right", True)],
         "Variable": [("Token", "name", False)]})

    define_ast(
        output_dir, "Stmt",
        {"Block": [("std::vector<std::shared_ptr<Stmt>>", "statements", False)],
         "Class": [("Token", "name", False), ("Expr", "superclass", True),
                   ("std::vector<std::shared_ptr<Function>>", "methods", False)],
         "Expression": [("Expr", "expression", True)],
         "Function": [("Token", "name", False),
                      ("std::vector<Token>", "parameters", False),
                      ("std::vector<std::shared_ptr<Stmt>>", "body", False)],
         "If": [("Expr", "condition", True), ("Stmt", "then_branch", True),
                ("Stmt", "else_branch", True)],
         "Print": [("Expr", "expression", True)],
         "Return": [("Token", "keyword", False), ("Expr", "value", True)],
         "Var": [("Token", "name", False), ("Expr", "initialiser", True)],
         "While": [("Expr", "condition", True), ("Stmt", "body", True)]},
        ["Expr.hpp"])
