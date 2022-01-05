#pragma once

#include <cctype>
#include <cppcoro/generator.hpp>
#include <string_view>
#include <type_traits>
#include <variant>

namespace fol::details::utils {
template <typename T, typename Label>
struct TypeWithLabel : public T {
  using T::T;
  TypeWithLabel(const TypeWithLabel &) = default;
  TypeWithLabel(TypeWithLabel &&) noexcept(noexcept(T(std::declval<T &&>()))) =
      default;

  TypeWithLabel &operator=(const TypeWithLabel &) = default;
  TypeWithLabel &operator=(TypeWithLabel &&) noexcept(
      noexcept(std::declval<T>() = std::declval<T &&>())) = default;

  ~TypeWithLabel() = default;
};

template <class T>
struct IsVariant : std::integral_constant<bool, false> {};

template <class... T>
struct IsVariant<std::variant<T...>> : std::integral_constant<bool, true> {};

inline std::string_view::size_type SkipWhiteSpaces(
    std::string_view::size_type from, std::string_view str) {
  while (from < str.size() && std::isspace(str[from])) {
    ++from;
  }
  return from;
}

inline std::string_view GetUntilWhiteSpace(std::string_view::size_type &from,
                                           std::string_view str) {
  std::string_view::size_type sv = from;
  while (from < str.size() && !std::isspace(str[from]) && str[from] != '(' &&
         str[from] != ')') {
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

template <class T>
inline T GetValueFromGenerator(cppcoro::generator<T> &generator) {
  return std::move(*generator.begin());
}

}  // namespace fol::details::utils

