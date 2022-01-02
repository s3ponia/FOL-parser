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
struct Coma : std::integral_constant<int, 8> {};

using Function =
    details::utils::TypeWithLabel<std::string, class FunctionLabel>;
using Variable =
    details::utils::TypeWithLabel<std::string, class VariableLabel>;
using Predicate =
    details::utils::TypeWithLabel<std::string, class PredicateLabel>;
using Constant =
    details::utils::TypeWithLabel<std::string, class ConstantLabel>;

using Lexeme =
    std::variant<OpenBracket, CloseBracket, Forall, Exists, And, Or, Implies,
                 Not, Coma, Function, Variable, Predicate, Constant>;

using LexemeGenerator = cppcoro::generator<const Lexeme>;

std::ostream &operator<<(std::ostream &os, const Lexeme &lexeme) {
  std::visit(
      details::utils::Overloaded{
          [&](OpenBracket) { os << '('; }, [&](CloseBracket) { os << ')'; },
          [&](Forall) { os << '@'; }, [&](Exists) { os << '?'; },
          [&](And) { os << "and"; }, [&](Or) { os << "or"; },
          [&](Implies) { os << "->"; }, [&](Not) { os << "not"; },
          [&](Coma) { os << ','; }, [&](auto &&v) { os << v; }},
      lexeme);
  return os;
}

struct LexerError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline LexemeGenerator Tokenize(std::string string) {
  int i = 0;
  while (i < string.size()) {
    i = details::utils::SkipWhiteSpaces(i, string);
    switch (string[i]) {
      case 'c':
        co_yield Constant{details::utils::GetUntilWhiteSpace(i, string)};
        break;
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
      case '-':  // ->
        if (string[i + 1] != '>') {
          throw LexerError{"Error in tokenizing"};
        }
        co_yield Implies{};
        i += 2;
        break;
      case ',':
        co_yield Coma{};
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
}

}  // namespace fol_parser::lexer

