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

#include "logging.hpp"
#include "Scanner.hpp"

#include <cctype>


namespace loxx
{

  Scanner::Scanner(std::string src)
      : start_(0), current_(0), line_(1), src_(std::move(src))
  {
    keywords_ =
        {{"and",    TokenType::And},
         {"class",  TokenType::Class},
         {"else",   TokenType::Else},
         {"false",  TokenType::False},
         {"for",    TokenType::For},
         {"fun",    TokenType::Fun},
         {"if",     TokenType::If},
         {"nil",    TokenType::Nil},
         {"or",     TokenType::Or},
         {"print",  TokenType::Print},
         {"return", TokenType::Return},
         {"super",  TokenType::Super},
         {"this",   TokenType::This},
         {"true",   TokenType::True},
         {"var",    TokenType::Var},
         {"while",  TokenType::While}
        };
  }


  const std::vector<Token>& Scanner::scan_tokens()
  {
    while (not is_at_end()) {
      start_ = current_;
      scan_token();
    }

    tokens_.emplace_back(TokenType::Eof, "", nullptr, line_);
    return tokens_;
  }


  void Scanner::scan_token()
  {
    const char c = advance();

    if (c == '(') {
      add_token(TokenType::LeftParen);
    }
    else if (c == ')') {
      add_token(TokenType::RightParen);
    }
    else if (c == '{') {
      add_token(TokenType::LeftBrace);
    }
    else if (c == '}') {
      add_token(TokenType::RightBrace);
    }
    else if (c == ',') {
      add_token(TokenType::Comma);
    }
    else if (c == '.') {
      add_token(TokenType::Dot);
    }
    else if (c == '-') {
      add_token(TokenType::Minus);
    }
    else if (c == '+') {
      add_token(TokenType::Plus);
    }
    else if (c == ';') {
      add_token(TokenType::SemiColon);
    }
    else if (c == '*') {
      add_token(TokenType::Star);
    }
    else if (c == '!') {
      add_token(match('=') ? TokenType::BangEqual : TokenType::Bang);
    }
    else if (c == '=') {
      add_token(match('=') ? TokenType::EqualEqual : TokenType::Equal);
    }
    else if (c == '<') {
      add_token(match('=') ? TokenType::LessEqual : TokenType::Less);
    }
    else if (c == '>') {
      add_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
    }
    else if (c == '/') {
      if (match('/')) {
        while (peek() != '\n' and not is_at_end()) {
          advance();
        }
      }
      else {
        add_token(TokenType::Slash);
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
        keywords_.count(text) == 0 ? TokenType::Identifier : keywords_[text];
    if (type == TokenType::True) {
      add_token(type, true);
    }
    else if (type == TokenType::False) {
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

    add_token(TokenType::String, src_.substr(start_ + 1, current_ - start_ - 2));
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
      add_token(TokenType::Number,
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
    tokens_.emplace_back(type, substr, nullptr, line_);
  }


  void Scanner::add_token(const TokenType type, Generic literal)
  {
    auto substr = src_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, substr, std::move(literal), line_);
  }
}