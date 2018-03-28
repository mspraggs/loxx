
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

#ifndef LOXX_SCANNER_HPP
#define LOXX_SCANNER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "Token.hpp"


namespace loxx
{
  class Scanner
  {
  public:
    explicit Scanner(std::string src);

    const std::vector<Token>& scan_tokens();

  private:
    void scan_token();

    void identifier();
    void string();
    void number();

    bool match(const char expected);
    char peek() const;
    char peek_next() const;

    bool is_alpha(const char c) const { return isalpha(c) != 0   or c == '_'; }
    bool is_digit(const char c) const { return isdigit(c) != 0; }
    bool is_alpha_numeric(const char c) const
    { return is_alpha(c) or is_digit(c); }
    bool is_at_end() const { return current_ >= src_.size(); }
    char advance();
    void add_token(const TokenType type);
    void add_token(const TokenType type, Value literal);

    unsigned int start_, current_, line_;

    std::string src_;
    std::vector<Token> tokens_;

    std::unordered_map<std::string, TokenType> keywords_;
  };
}

#endif //LOXX_SCANNER_HPP
