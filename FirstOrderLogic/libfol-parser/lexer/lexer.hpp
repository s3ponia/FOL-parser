#pragma once

#include <cppcoro/generator.hpp>
#include <details/utils/utility.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace fol::lexer {
struct OpenBracket : std::integral_constant<int, 0> {};
struct CloseBracket : std::integral_constant<int, 1> {};
struct Forall : std::integral_constant<int, 2> {};
struct Exists : std::integral_constant<int, 3> {};
struct And : std::integral_constant<int, 4> {};
struct Or : std::integral_constant<int, 5> {};
struct Implies : std::integral_constant<int, 6> {};
struct Not : std::integral_constant<int, 7> {};
struct Comma : std::integral_constant<int, 8> {};
struct Dot : std::integral_constant<int, 9> {};
struct EPS : std::integral_constant<int, 10> {};

using Function =
    details::utils::TypeWithLabel<std::string, class FunctionLabel>;
using Variable =
    details::utils::TypeWithLabel<std::string, class VariableLabel>;
using Predicate =
    details::utils::TypeWithLabel<std::string, class PredicateLabel>;
using Constant =
    details::utils::TypeWithLabel<std::string, class ConstantLabel>;

using Lexeme = std::variant<EPS, OpenBracket, CloseBracket, Forall, Exists, And,
                            Or, Implies, Not, Comma, Dot, Function, Variable,
                            Predicate, Constant>;

using LexemeGenerator = cppcoro::generator<const Lexeme>;

inline namespace literals {
inline Constant operator""_c(const char *str, std::size_t) {
  std::string res = std::string{"c"} + str;
  return res.c_str();
}
inline Variable operator""_v(const char *str, std::size_t) {
  std::string res = std::string{"v"} + str;
  return res.c_str();
}
inline Predicate operator""_p(const char *str, std::size_t) {
  std::string res = std::string{"p"} + str;
  return res.c_str();
}
inline Function operator""_f(const char *str, std::size_t) {
  std::string res = std::string{"f"} + str;
  return res.c_str();
}
}  // namespace literals

inline std::ostream &operator<<(std::ostream &os, OpenBracket) {
  return os << "(";
}

inline std::ostream &operator<<(std::ostream &os, CloseBracket) {
  return os << ")";
}

inline std::ostream &operator<<(std::ostream &os, Forall) { return os << "@"; }

inline std::ostream &operator<<(std::ostream &os, Exists) { return os << "?"; }

inline std::ostream &operator<<(std::ostream &os, And) { return os << "and"; }

inline std::ostream &operator<<(std::ostream &os, Or) { return os << "or"; }

inline std::ostream &operator<<(std::ostream &os, Implies) {
  return os << "->";
}

inline std::ostream &operator<<(std::ostream &os, Not) { return os << "~"; }

inline std::ostream &operator<<(std::ostream &os, Comma) { return os << ","; }

inline std::ostream &operator<<(std::ostream &os, Dot) { return os << "."; }

inline std::ostream &operator<<(std::ostream &os, EPS) { return os; }

inline std::ostream &operator<<(std::ostream &os, const Lexeme &lexeme) {
  std::visit([&](auto &&v) { os << v; }, lexeme);
  return os;
}

struct LexerError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline std::string::size_type i = 0;

inline LexemeGenerator Tokenize(std::string string) {
  i = 0;
  while (i < string.size()) {
    i = details::utils::SkipWhiteSpaces(i, string);
    switch (string[i]) {
      case 'v':
        co_yield Variable{details::utils::GetUntilWhiteSpace(i, string)};
        break;
      case 'f':
        co_yield Function{details::utils::GetUntilWhiteSpace(i, string)};
        break;
      case 'p':
        co_yield Predicate{details::utils::GetUntilWhiteSpace(i, string)};
        break;
      case '@':
        co_yield Forall{};
        ++i;
        break;
      case '?':
        co_yield Exists{};
        ++i;
        break;
      case '(':
        co_yield OpenBracket{};
        ++i;
        break;
      case ')':
        co_yield CloseBracket{};
        ++i;
        break;
      case 'a':  // and
        if (string[i + 1] != 'n' || string[i + 2] != 'd') {
          throw LexerError{"Error in tokenizing"};
        }
        co_yield And{};
        i += 3;
        break;
      case '~':
        co_yield Not{};
        ++i;
        break;
      case '-':  // ->
        if (string[i + 1] != '>') {
          throw LexerError{"Error in tokenizing"};
        }
        co_yield Implies{};
        i += 2;
        break;
      case ',':
        co_yield Comma{};
        ++i;
        break;
      case '.':
        co_yield Dot{};
        ++i;
        break;
      case 'o':  // or
        if (string[i + 1] != 'r') {
          throw LexerError{"Error in tokenizing"};
        }
        co_yield Or{};
        i += 2;
        break;
      default:
        throw LexerError{"Unhandled lexem"};
    }
  }
  for (;;) {
    co_yield EPS{};
  }
}

}  // namespace fol::lexer

