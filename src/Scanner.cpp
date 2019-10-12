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

#include "globals.hpp"
#include "logging.hpp"
#include "Scanner.hpp"
#include "ObjectTracker.hpp"

#include <cctype>


namespace loxx
{

  Scanner::Scanner(std::string src)
      : start_(0), current_(0), line_(1), src_(std::move(src))
  {
    keywords_ =
        {{"and",    TokenType::AND},
         {"class",  TokenType::CLASS},
         {"else",   TokenType::ELSE},
         {"false",  TokenType::FALSE},
         {"for",    TokenType::FOR},
         {"fun",    TokenType::FUN},
         {"if",     TokenType::IF},
         {"nil",    TokenType::NIL},
         {"or",     TokenType::OR},
         {"print",  TokenType::PRINT},
         {"return", TokenType::RETURN},
         {"super",  TokenType::SUPER},
         {"this",   TokenType::THIS},
         {"true",   TokenType::TRUE},
         {"var",    TokenType::VAR},
         {"while",  TokenType::WHILE}
        };
  }


  const std::vector<Token>& Scanner::scan_tokens()
  {
    while (not is_at_end()) {
      start_ = current_;
      scan_token();
    }

    tokens_.emplace_back(TokenType::END_OF_FILE, "", line_);
    return tokens_;
  }


  void Scanner::scan_token()
  {
    const char c = advance();

    if (c == '(') {
      add_token(TokenType::LEFT_PAREN);
    }
    else if (c == ')') {
      add_token(TokenType::RIGHT_PAREN);
    }
    else if (c == '{') {
      add_token(TokenType::LEFT_BRACE);
    }
    else if (c == '}') {
      add_token(TokenType::RIGHT_BRACE);
    }
    else if (c == ',') {
      add_token(TokenType::COMMA);
    }
    else if (c == '.') {
      add_token(TokenType::DOT);
    }
    else if (c == '-') {
      add_token(TokenType::MINUS);
    }
    else if (c == '+') {
      add_token(TokenType::PLUS);
    }
    else if (c == ';') {
      add_token(TokenType::SEMI_COLON);
    }
    else if (c == '*') {
      add_token(TokenType::STAR);
    }
    else if (c == '!') {
      add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    }
    else if (c == '=') {
      add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    }
    else if (c == '<') {
      add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    }
    else if (c == '>') {
      add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    }
    else if (c == '/') {
      if (match('/')) {
        while (peek() != '\n' and not is_at_end()) {
          advance();
        }
      }
      else {
        add_token(TokenType::SLASH);
      }
    }
    else if (c == ' ' or c == '\r' or c == '\t') {

    }
    else if (c == '\n') {
      line_ += 1;
    }
    else if (c == '"') {
      string();
    }
    else if (is_digit(c)) {
      number();
    }
    else if (is_alpha(c)) {
      identifier();
    }
    else {
      error(line_, std::string("Unexpected character: '") + c + "'.");
    }
  }


  void Scanner::identifier()
  {
    while (is_alpha_numeric(peek())) {
      advance();
    }

    const auto text = src_.substr(start_, current_ - start_);
    const TokenType type =
        keywords_.count(text) == 0 ? TokenType::IDENTIFIER : keywords_[text];
    if (type == TokenType::TRUE) {
      add_token(type, true);
    }
    else if (type == TokenType::FALSE) {
      add_token(type, false);
    }
    else {
      add_token(type);
    }
  }


  void Scanner::string()
  {
    while (peek() != '"' and not is_at_end()) {
      if (peek() == '\n') {
        line_ += 1;
      }
      advance();
    }

    if (is_at_end()) {
      error(line_, "Unterminated string.");
    }

    advance();

    const auto string_obj = make_string(
        src_.substr(start_ + 1, current_ - start_ - 2));
    add_token(TokenType::STRING, Value(InPlace<ObjectPtr>(), string_obj));
  }


  void Scanner::number()
  {
    while (is_digit(peek())) {
      advance();
    }

    if (peek() == '.' and is_digit(peek_next())) {
      advance();

      while (is_digit(peek())) {
        advance();
      }
    }

    try {
      add_token(TokenType::NUMBER,
                std::stod(src_.substr(start_, current_ - start_)));
    }
    catch (const std::out_of_range& e) {
      error(line_, "Unable to parse number: out of range");
    }
  }


  bool Scanner::match(const char expected)
  {
    if (is_at_end()) {
      return false;
    }
    if (src_[current_] != expected) {
      return false;
    }

    current_ += 1;
    return true;
  }


  char Scanner::peek() const
  {
    if (is_at_end()) {
      return '\0';
    }
    else {
      return src_[current_];
    }
  }


  char Scanner::peek_next() const
  {
    if (current_ + 1 >= src_.size()) {
      return '\0';
    }
    return src_[current_ + 1];
  }


  char Scanner::advance()
  {
    current_ += 1;
    return src_[current_ - 1];
  }


  void Scanner::add_token(const TokenType type)
  {
    auto substr = src_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, substr, line_);
  }


  void Scanner::add_token(const TokenType type, Value literal)
  {
    auto substr = src_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, substr, std::move(literal), line_);
  }
}
