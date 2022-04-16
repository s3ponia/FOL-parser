#pragma once

#include <cctype>
#include <cppcoro/generator.hpp>
#include <string_view>
#include <tuple>
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

  T &base() { return *this; }

  const T &base() const { return *this; }

  ~TypeWithLabel() = default;
};

template <typename>
struct ToTuple;

template <template <typename...> typename TupleType, typename... Args>
struct ToTuple<TupleType<Args...>> : std::type_identity<std::tuple<Args...>> {};

template <std::size_t... Indices, typename... TupleElems>
auto SubTupleImpl(std::index_sequence<Indices...>,
                  const std::tuple<TupleElems...> &t) {
  return std::make_tuple(std::get<Indices>(t)...);
}

template <typename IndexSequence, typename TupleType>
struct SubTuple
    : std::type_identity<decltype(SubTupleImpl(IndexSequence{}, TupleType{}))> {
};

template <template <typename...> typename Type, typename Tuple>
struct Apply;

template <template <typename...> typename Type, typename... TupleElems>
struct Apply<Type, std::tuple<TupleElems...>>
    : std::type_identity<Type<TupleElems...>> {};

template <typename T, typename... TypeList>
concept InList = (std::is_same_v<T, TypeList> || ...);

template <typename T>
concept HasMemberData = requires(T t) {
  t.data;
};

template <typename T>
concept Dereferencable = requires(T t) {
  *t;
};

template <typename T>
concept PtrPairType = requires(T t) {
  t->first;
  t->second;
};

template <typename T>
concept PairType = requires(T t) {
  t.first;
  t.second;
};

template <class T>
struct IsVariant : std::integral_constant<bool, false> {};

template <class... T>
struct IsVariant<std::variant<T...>> : std::integral_constant<bool, false> {};

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
         str[from] != ')' && str[from] != ',' && str[from] != '.' &&
         str[from] != '?' && str[from] != '@' && str[from] != '~' &&
         str[from] != '-') {
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

