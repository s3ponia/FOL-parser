#pragma once

#include <cctype>
#include <string_view>

namespace fol_parser::details::utils {
template <typename T, typename Label>
struct TypeWithLabel : public T {
  using T::T;
};

int SkipWhiteSpaces(int from, std::string_view str) {
  while (from < str.size() && std::isspace(str[from])) {
    ++from;
  }
  return from;
}

inline std::string_view GetUntilWhiteSpace(int &from, std::string_view str) {
  int sv = from;
  while (from < str.size() && !std::isspace(str[from])) {
    ++from;
  }
  return std::string_view{str.begin() + sv, from - sv};
}

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}  // namespace fol_parser::details::utils

