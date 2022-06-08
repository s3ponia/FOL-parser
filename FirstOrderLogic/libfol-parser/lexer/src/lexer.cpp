#include <libfol-parser/lexer/lexer.hpp>

namespace fol::lexer {
inline namespace literals {
Constant operator""_c(const char *str, std::size_t) {
  std::string res = std::string{"c"} + str;
  return res.c_str();
}
Variable operator""_v(const char *str, std::size_t) {
  std::string res = std::string{"v"} + str;
  return res.c_str();
}
Predicate operator""_p(const char *str, std::size_t) {
  std::string res = std::string{"p"} + str;
  return res.c_str();
}
Function operator""_f(const char *str, std::size_t) {
  std::string res = std::string{"f"} + str;
  return res.c_str();
}
}  // namespace literals

std::ostream &operator<<(std::ostream &os, OpenBracket) { return os << "("; }

std::ostream &operator<<(std::ostream &os, CloseBracket) { return os << ")"; }

std::ostream &operator<<(std::ostream &os, Forall) { return os << "@"; }

std::ostream &operator<<(std::ostream &os, Exists) { return os << "?"; }

std::ostream &operator<<(std::ostream &os, And) { return os << "and"; }

std::ostream &operator<<(std::ostream &os, Or) { return os << "or"; }

std::ostream &operator<<(std::ostream &os, Implies) { return os << "->"; }

std::ostream &operator<<(std::ostream &os, Not) { return os << "~"; }

std::ostream &operator<<(std::ostream &os, Comma) { return os << ","; }

std::ostream &operator<<(std::ostream &os, Dot) { return os << "."; }

std::ostream &operator<<(std::ostream &os, EPS) { return os; }

std::ostream &operator<<(std::ostream &os, const Lexeme &lexeme) {
  std::visit([&](auto &&v) { os << v; }, lexeme);
  return os;
}

LexemeGenerator Tokenize(std::string string) {
  i = 0;
  while (i < string.size()) {
    i = details::utils::SkipWhiteSpaces(i, string);
    if (i >= string.size()) {
      break;
    }
    switch (string[i]) {
      case 'v':
        co_yield Variable{details::utils::GetUntilWhiteSpace(i, string)};
        break;
      case 'c':
        co_yield Constant{details::utils::GetUntilWhiteSpace(i, string)};
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
        throw LexerError{std::string{"Unhandled lexem: \'"} + string[i] + "'"};
    }
  }
  for (;;) {
    co_yield EPS{};
  }
}

}  // namespace fol::lexer
