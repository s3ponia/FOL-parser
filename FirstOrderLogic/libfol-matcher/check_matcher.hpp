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
  bool operator()(const auto &t) const { return check(t); }
  bool check(auto &&) const { return true; }
};

template <typename T>
struct CheckerCrtp {
  template <typename A>
  bool operator()(const A &a) const {
    return check(a);
  }

  bool check(const auto &a) const { return static_cast<T>(*this).check(a); }
};

template <typename Lhs, typename Rhs>
struct Or : CheckerCrtp<Or<Lhs, Rhs>> {
  bool check(const auto &a) const { return Lhs{}(a) || Rhs{}(a); }
};

template <typename Type, typename Checker>
struct SimpleCompoundCheckerImpl
    : CheckerCrtp<SimpleCompoundCheckerImpl<Type, Checker>> {
  template <class T>
  bool checkEps(const T &) const {
    return false;
  }
  bool checkEps(const lexer::EPS &) const { return true; }
  template <typename... Args>
  bool checkEps(const std::variant<Args...> &t) const {
    return std::visit([this](const auto &a) { return checkEps(a); }, t);
  }
  bool checkEps(const details::utils::HasMemberData auto &t) const {
    return checkEps(t.data);
  }

  bool check(const details::utils::PairType auto &t) const {
    return checkEps(t.second) && check(t.first);
  }
  bool check(const details::utils::Dereferencable auto &t) const {
    return check(*t);
  }
  bool check(const Type &t) const {
    if constexpr (details::utils::HasMemberData<Type>) {
      return Checker{}(t.data);
    } else {
      return Checker{}(t);
    }
  }
  template <typename... Args>
  bool check(const std::variant<Args...> &v) const {
    return std::visit([this](const auto &a) { return check(a); }, v);
  }
  template <typename T>
  bool check(const T &t) const {
    if constexpr (details::utils::InList<
                      T, parser::NotFormula, parser::ForallFormula,
                      parser::ExistsFormula, parser::PredicateFormula>) {
      return false;
    }
    if constexpr (!details::utils::HasMemberData<T>) {
      if constexpr (details::utils::Dereferencable<T>) {
        return check(*t);
      }
      return false;
    } else {
      return check(t.data);
    }
  }
};

template <typename T = AlwaysTrueChecker>
using CheckBracketsCompound =
    SimpleCompoundCheckerImpl<parser::BracketFormula, T>;

using CheckBrackets = CheckBracketsCompound<>;

template <typename T>
using SkipBrackets = Or<CheckBracketsCompound<T>, T>;

template <typename T, typename Checker>
using SimpleCompoundChecker =
    SkipBrackets<SimpleCompoundCheckerImpl<T, Checker>>;

template <typename LhsChecker, typename RhsChecker>
struct BasicPairChecker
    : CheckerCrtp<BasicPairChecker<LhsChecker, RhsChecker>> {
  bool check(const details::utils::PtrPairType auto &t) const {
    return check(*t);
  }
  bool check(const details::utils::PairType auto &t) const {
    return LhsChecker{}(t.first) && RhsChecker{}(t.second);
  }
};

template <typename PairType, typename LhsChecker, typename RhsChecker>
struct PairCompoundChecker
    : CheckerCrtp<PairCompoundChecker<PairType, LhsChecker, RhsChecker>> {
  template <typename T>
  bool check(const T &t) const {
    return SimpleCompoundChecker<PairType,
                                 BasicPairChecker<LhsChecker, RhsChecker>>{}(t);
  }
};

template <typename T>
using SimpleChecker = SimpleCompoundChecker<T, AlwaysTrueChecker>;

using CheckImpl = SimpleChecker<parser::ImplicationFormula>;

template <typename Checker>
using SimpleCompoundCheckImpl =
    SimpleCompoundChecker<parser::ImplicationFormula, Checker>;

template <typename Lhs, typename Rhs>
using CompoundCheckImpl = SimpleCompoundCheckImpl<PairCompoundChecker<
    std::pair<parser::DisjunctionFormula, parser::ImplicationFormula>, Lhs,
    Rhs>>;

template <typename T = AlwaysTrueChecker>
using CheckDisjCompound = SimpleCompoundChecker<parser::DisjunctionFormula, T>;

using CheckDisj = CheckDisjCompound<>;

template <typename T>
using CompoundSnglePairDisjPrimeCheck = SimpleCompoundChecker<
    std::pair<parser::ConjunctionFormula, parser::DisjunctionPrimeFormula>, T>;

template <typename Lhs, typename Rhs>
using CompoundCheckPairDisjPrime = PairCompoundChecker<
    std::pair<parser::ConjunctionFormula, parser::DisjunctionPrimeFormula>, Lhs,
    Rhs>;

template <typename...>
struct CompoundCheckDisj;

template <typename Lhs, typename Rhs>
struct CompoundCheckDisj<Lhs, Rhs>
    : CompoundCheckPairDisjPrime<Lhs, CompoundSnglePairDisjPrimeCheck<Rhs>> {};

template <typename Head, typename... Args>
struct CompoundCheckDisj<Head, Args...>
    : CompoundCheckPairDisjPrime<Head, CompoundCheckDisj<Args...>> {};

template <typename T = AlwaysTrueChecker>
using CheckConjCompound = SimpleCompoundChecker<parser::ConjunctionFormula, T>;

using CheckConj = CheckConjCompound<>;

template <typename T>
using CompoundSnglePairConjPrimeCheck = SimpleCompoundChecker<
    std::pair<parser::UnaryFormula, parser::ConjunctionPrimeFormula>, T>;

template <typename Lhs, typename Rhs>
using CompoundCheckPairConjPrime = PairCompoundChecker<
    std::pair<parser::UnaryFormula, parser::ConjunctionPrimeFormula>, Lhs, Rhs>;

template <typename...>
struct CompoundCheckConj;

template <typename Lhs, typename Rhs>
struct CompoundCheckConj<Lhs, Rhs>
    : CompoundCheckPairConjPrime<Lhs, CompoundSnglePairConjPrimeCheck<Rhs>> {};

template <typename Head, typename... Args>
struct CompoundCheckConj<Head, Args...>
    : CompoundCheckPairConjPrime<Head, CompoundCheckConj<Args...>> {};

using CheckUnary = SimpleChecker<parser::UnaryFormula>;

template <typename Checker>
using CompoundCheckUnary = SimpleCompoundChecker<parser::UnaryFormula, Checker>;

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
    PairCompoundChecker<std::pair<std::string, parser::ImplicationFormula>,
                        AlwaysTrueChecker, Checker>>;

using CheckExists = SimpleChecker<parser::ExistsFormula>;

template <typename Checker>
using CompoundCheckExists = SimpleCompoundChecker<
    parser::ExistsFormula,
    PairCompoundChecker<std::pair<std::string, parser::ImplicationFormula>,
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
template <typename... T>
inline CompoundCheckDisj<T...> Disj(T...) {
  return {};
}
template <typename... T>
inline CompoundCheckConj<T...> Conj(T...) {
  return {};
}

DECL_CHECK_PAIR_FAC_FUN(CheckConj, Conj)

inline CheckUnary Unary() { return {}; }
inline AlwaysTrueChecker Anything() { return {}; }

DECL_CHECK_FAC_FUN(CheckBrackets, Brackets)
DECL_CHECK_FAC_FUN(CheckNot, Not)
DECL_CHECK_FAC_FUN(CheckForall, Forall)
DECL_CHECK_FAC_FUN(CheckExists, Exists)

inline CheckPred Pred() { return {}; }

}  // namespace fol::matcher::check
