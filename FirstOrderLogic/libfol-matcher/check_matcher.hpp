#pragma once

#include <functional>
#include <libfol-parser/parser/types.hpp>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

#include "details/utils/utility.hpp"
#include "libfol-parser/lexer/lexer.hpp"

namespace fol::matcher::check {
struct AlwaysTrueChecker {
  template <typename T>
  bool operator()(const T &t) const {
    return check(t);
  }
  template <typename T>
  bool check(T &&) const {
    return true;
  }
};

template <typename T>
concept HasMemberData = requires(T t) {
  t.data;
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

template <typename T>
struct CheckerCrtp {
  template <typename A>
  bool operator()(const A &a) const {
    return check(a);
  }
  template <typename A>
  bool check(const A &a) const {
    return static_cast<T>(*this).check(a);
  }
};

template <typename Type, typename Checker>
struct SimpleCompoundChecker
    : CheckerCrtp<SimpleCompoundChecker<Type, Checker>> {
  bool check(lexer::EPS) const { return true; }
  bool check(const PairType auto &t) const { return check(&t); }
  bool check(const PtrPairType auto &t) const {
    return check(t->first) || check(t->second);
  }
  bool check(const Type &t) const { return Checker{}(t); }
  template <typename... Args>
  bool check(const std::variant<Args...> &v) const {
    return std::visit([this](const auto &a) { return check(a); }, v);
  }
  template <typename T>
  bool check(const T &t) const {
    if constexpr (!HasMemberData<T>) {
      return false;
    } else {
      return check(t.data);
    }
  }
};

template <typename PairType, typename LhsChecker, typename RhsChecker>
struct PairCompoundChecker {
  template <typename T>
  bool operator()(const T &t) const {
    return check(t);
  }
  template <typename T>
  bool check(const T &t) const {
    if constexpr (std::is_same_v<std::decay_t<decltype(t.data)>, PairType>) {
      return LhsChecker{}.check(t.data.first) &&
             RhsChecker{}.check(t.data.second);
    } else if constexpr (details::utils::IsVariant<decltype(t.data)>{}) {
      return std::visit(
          details::utils::Overloaded{[](const PairType &t) {
                                       return LhsChecker{}(t.first) &&
                                              RhsChecker{}(t.second);
                                     },
                                     [](auto &&) { return false; }},
          t.data);
    } else {
      return false;
    }
  }
};

template <typename PairType, typename LhsChecker, typename RhsChecker>
struct PairCompoundChecker<std::unique_ptr<PairType>, LhsChecker, RhsChecker> {
  template <typename T>
  bool operator()(const T &t) const {
    return check(t);
  }
  template <typename T>
  bool check(const T &t) const {
    if constexpr (std::is_same_v<std::decay_t<decltype(t.data)>, PairType>) {
      return LhsChecker{}.check(t.data.first) &&
             RhsChecker{}.check(t.data.second);
    } else if constexpr (details::utils::IsVariant<decltype(t.data)>{}) {
      return std::visit(
          details::utils::Overloaded{[](const std::unique_ptr<PairType> &t) {
                                       return LhsChecker{}(t->first) &&
                                              RhsChecker{}(t->second);
                                     },
                                     [](auto &&) { return false; }},
          t.data);
    } else {
      return false;
    }
  }
};

template <typename T>
using SimpleChecker = SimpleCompoundChecker<T, AlwaysTrueChecker>;

using CheckImpl = SimpleChecker<parser::ImplicationFormula>;

template <typename Checker>
using SimpleCompoundCheckImpl =
    SimpleCompoundChecker<parser::ImplicationFormula, Checker>;

template <typename Lhs, typename Rhs>
using CompoundCheckImpl = SimpleCompoundCheckImpl<
    PairCompoundChecker<std::unique_ptr<std::pair<parser::DisjunctionFormula,
                                                  parser::ImplicationFormula>>,
                        Lhs, Rhs>>;

using CheckDisj = SimpleChecker<parser::DisjunctionFormula>;

template <typename Lhs, typename Rhs>
using CompoundCheckDisj = SimpleCompoundChecker<
    parser::DisjunctionFormula,
    PairCompoundChecker<
        std::pair<parser::ConjunctionFormula, parser::DisjunctionPrimeFormula>,
        Lhs,
        PairCompoundChecker<std::pair<parser::ConjunctionFormula,
                                      parser::DisjunctionPrimeFormula>,
                            Rhs, AlwaysTrueChecker>>>;

using CheckConj = SimpleChecker<parser::ConjunctionFormula>;

template <typename Lhs, typename Rhs>
using CompoundCheckConj = SimpleCompoundChecker<
    parser::DisjunctionFormula,
    PairCompoundChecker<
        std::unique_ptr<std::pair<parser::ConjunctionFormula,
                                  parser::DisjunctionPrimeFormula>>,
        Lhs,
        PairCompoundChecker<
            std::pair<parser::UnaryFormula, parser::ConjuctionPrimeFormula>,
            Rhs, AlwaysTrueChecker>>>;

using CheckUnary = SimpleChecker<parser::UnaryFormula>;

template <typename Checker>
using CompoundCheckUnary = SimpleCompoundChecker<parser::UnaryFormula, Checker>;

using CheckBrackets = SimpleChecker<parser::BracketFormula>;

template <typename Checker>
using CompoundCheckBrackets =
    SimpleCompoundChecker<parser::BracketFormula, Checker>;

using CheckNot = SimpleChecker<parser::NotFormula>;

template <typename Checker>
using CompoundCheckNot = SimpleCompoundChecker<parser::NotFormula, Checker>;

using CheckForall = SimpleChecker<parser::ForallFormula>;

template <typename Checker>
using CompoundCheckForall = SimpleCompoundChecker<
    parser::ForallFormula,
    PairCompoundChecker<std::pair<lexer::Variable, parser::ImplicationFormula>,
                        AlwaysTrueChecker, Checker>>;

using CheckExists = SimpleChecker<parser::ExistsFormula>;

template <typename Checker>
using CompoundCheckExists = SimpleCompoundChecker<
    parser::ExistsFormula,
    PairCompoundChecker<std::pair<lexer::Variable, parser::ImplicationFormula>,
                        AlwaysTrueChecker, Checker>>;

using CheckPred = SimpleChecker<parser::PredicateFormula>;

#define DECL_PAIR_FAC_FUN(TemplateName, FunName)    \
  template <typename Lhs, typename Rhs>             \
  inline TemplateName<Lhs, Rhs> FunName(Lhs, Rhs) { \
    return {};                                      \
  }

#define DECL_FAC_FUN(TemplateName, FunName) \
  template <typename Lhs>                   \
  inline TemplateName<Lhs> FunName(Lhs) {   \
    return {};                              \
  }

#define DECL_CHECK_FAC_FUN(CheckName, FunName) \
  inline CheckName FunName() { return {}; }    \
  DECL_FAC_FUN(Compound##CheckName, FunName)

#define DECL_CHECK_PAIR_FAC_FUN(CheckName, FunName) \
  inline CheckName FunName() { return {}; }         \
  DECL_PAIR_FAC_FUN(Compound##CheckName, FunName)

DECL_CHECK_PAIR_FAC_FUN(CheckImpl, Impl)

template <typename T>
inline SimpleCompoundCheckImpl<T> Impl(T) {
  return {};
}

DECL_CHECK_PAIR_FAC_FUN(CheckDisj, Disj)
DECL_CHECK_PAIR_FAC_FUN(CheckConj, Conj)

inline CheckUnary Unary() { return {}; }

DECL_CHECK_FAC_FUN(CheckBrackets, Brackets)
DECL_CHECK_FAC_FUN(CheckNot, Not)
DECL_CHECK_FAC_FUN(CheckForall, Forall)
DECL_CHECK_FAC_FUN(CheckExists, Exists)

inline CheckPred Pred() { return {}; }

}  // namespace fol::matcher::check
