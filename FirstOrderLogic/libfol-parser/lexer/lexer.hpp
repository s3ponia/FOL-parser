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
Constant operator""_c(const char *str, std::size_t);
Variable operator""_v(const char *str, std::size_t);
Predicate operator""_p(const char *str, std::size_t);
Function operator""_f(const char *str, std::size_t);
}  // namespace literals

std::ostream &operator<<(std::ostream &os, OpenBracket);

std::ostream &operator<<(std::ostream &os, CloseBracket);

std::ostream &operator<<(std::ostream &os, Forall);

std::ostream &operator<<(std::ostream &os, Exists);

std::ostream &operator<<(std::ostream &os, And);

std::ostream &operator<<(std::ostream &os, Or);

std::ostream &operator<<(std::ostream &os, Implies);

std::ostream &operator<<(std::ostream &os, Not);

std::ostream &operator<<(std::ostream &os, Comma);

std::ostream &operator<<(std::ostream &os, Dot);

std::ostream &operator<<(std::ostream &os, EPS);

std::ostream &operator<<(std::ostream &os, const Lexeme &lexeme);

struct LexerError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline std::string::size_type i = 0;

LexemeGenerator Tokenize(std::string string);

}  // namespace fol::lexer

