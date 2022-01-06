#pragma once

#include <libfol-parser/lexer/lexer.hpp>
#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>

namespace fol::parser {
struct ImplicationFormula;
struct DisjunctionFormula;
struct DisjunctionPrimeFormula;
struct ConjunctionFormula;
struct ConjunctionPrimeFormula;
struct UnaryFormula;
struct PredicateFormula;
struct FunctionFormula;
struct Term;
struct TermList;

using FolFormula = ImplicationFormula;

struct FunctionFormula {
  std::unique_ptr<std::pair<lexer::Function, TermList>> data;
};

struct Term {
  Term(lexer::Constant c) : data(c) {}
  Term(lexer::Variable v) : data(v) {}
  Term(FunctionFormula f) : data(std::move(f)) {}

  std::variant<lexer::Constant, lexer::Variable, FunctionFormula> data;
};

inline namespace literals {
inline Term operator""_t(const char* str, std::size_t) {
  switch (str[0]) {
    case 'c':
      return {lexer::Constant{str}};
    case 'v':
      return {lexer::Variable{str}};
    default:
      throw std::runtime_error{"Unhandled variant in literal _p"};
  }
}
}  // namespace literals

struct TermListPrime {
  std::variant<std::unique_ptr<TermList>, lexer::EPS> data;
};

struct TermList {
  TermList(Term t, TermListPrime term_list_prime = TermListPrime{lexer::EPS{}})
      : data{std::move(t), std::move(term_list_prime)} {}
  std::pair<Term, TermListPrime> data;
};

struct ConjunctionPrimeFormula {
  std::variant<
      std::unique_ptr<std::pair<UnaryFormula, ConjunctionPrimeFormula>>,
      lexer::EPS>
      data;
};

struct ConjunctionFormula {
  std::unique_ptr<std::pair<UnaryFormula, ConjunctionPrimeFormula>> data;
};

struct DisjunctionPrimeFormula {
  std::variant<
      std::unique_ptr<std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>,
      lexer::EPS>
      data;
};

struct DisjunctionFormula {
  DisjunctionFormula(ConjunctionFormula c_formula,
                     DisjunctionPrimeFormula disj_prime =
                         DisjunctionPrimeFormula{lexer::EPS{}})
      : data{std::move(c_formula), std::move(disj_prime)} {}
  DisjunctionFormula(
      std::pair<ConjunctionFormula, DisjunctionPrimeFormula> pair)
      : data(std::move(pair)) {}

  std::pair<ConjunctionFormula, DisjunctionPrimeFormula> data;
};

struct ImplicationFormula {
  std::variant<
      DisjunctionFormula,
      std::unique_ptr<std::pair<DisjunctionFormula, ImplicationFormula>>>
      data;
};

struct BracketFormula {
  ImplicationFormula data;
};

struct NotFormula {
  std::unique_ptr<UnaryFormula> data;
};

struct ForallFormula {
  std::pair<std::string, ImplicationFormula> data;
};

struct ExistsFormula {
  std::pair<std::string, ImplicationFormula> data;
};

struct PredicateFormula {
  std::pair<lexer::Predicate, TermList> data;
};

struct UnaryFormula {
  operator DisjunctionFormula() && {
    return DisjunctionFormula{ConjunctionFormula{
        std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
            std::move(*this), ConjunctionPrimeFormula{lexer::EPS{}})}};
  }
  operator ImplicationFormula() && {
    return ImplicationFormula{
        static_cast<DisjunctionFormula>(std::move(*this))};
  }
  std::variant<BracketFormula, NotFormula, ForallFormula, ExistsFormula,
               PredicateFormula>
      data;
};

inline BracketFormula MakeBrackets(parser::ImplicationFormula impl) {
  return BracketFormula{std::move(impl)};
}

inline ExistsFormula MakeExists(std::string var,
                                parser::ImplicationFormula impl) {
  return {{std::move(var), std::move(impl)}};
}

inline ForallFormula MakeForall(lexer::Variable var,
                                parser::ImplicationFormula impl) {
  return {{std::move(var), std::move(impl)}};
}

inline NotFormula MakeNot(UnaryFormula unary) {
  return {std::make_unique<UnaryFormula>(std::move(unary))};
}

inline ConjunctionFormula MakeConj(UnaryFormula unary) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(unary), ConjunctionPrimeFormula{lexer::EPS{}})};
}

inline ConjunctionFormula MakeConj(UnaryFormula unary,
                                   ConjunctionFormula conj) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(unary), ConjunctionPrimeFormula{std::move(conj.data)})};
}

inline DisjunctionFormula MakeDisj(ConjunctionFormula conj) {
  return {std::move(conj), DisjunctionPrimeFormula{lexer::EPS{}}};
}

inline DisjunctionFormula MakeDisj(ConjunctionFormula conj,
                                   DisjunctionFormula disj) {
  return {std::move(conj),
          DisjunctionPrimeFormula{std::make_unique<
              std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
              std::move(disj.data))}};
}

inline ImplicationFormula MakeImpl(DisjunctionFormula disj) {
  return {std::move(disj)};
}

inline ImplicationFormula MakeImpl(DisjunctionFormula disj,
                                   ImplicationFormula impl) {
  return {std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
      std::move(disj), std::move(impl))};
}

inline ImplicationFormula operator>>=(UnaryFormula disj,
                                      ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

inline DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                                     ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{
      std::move(conj_lhs),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(conj_rhs), DisjunctionPrimeFormula{lexer::EPS{}})}};
}

inline ConjunctionFormula operator&&(UnaryFormula fol_lhs,
                                     UnaryFormula fol_rhs) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(fol_lhs),
      ConjunctionPrimeFormula{
          std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
              std::move(fol_rhs), ConjunctionPrimeFormula{lexer::EPS{}})})};
}

inline FunctionFormula operator*(lexer::Function function, TermList term_list) {
  return {std::make_unique<std::pair<lexer::Function, TermList>>(
      std::move(function), std::move(term_list))};
}

inline TermList operator|=(Term term, TermList term_list) {
  return TermList{
      std::move(term),
      TermListPrime{std::make_unique<TermList>(std::move(term_list))}};
}

inline UnaryFormula operator*(lexer::Predicate pred, TermList term_list) {
  return {PredicateFormula{{std::move(pred), std::move(term_list)}}};
}

inline UnaryFormula ForAll(lexer::Variable var, ImplicationFormula impl) {
  return {ForallFormula{{std::move(var), std::move(impl)}}};
}

inline UnaryFormula Exists(lexer::Variable var, ImplicationFormula impl) {
  return {ExistsFormula{{std::move(var), std::move(impl)}}};
}

inline UnaryFormula B$(ImplicationFormula impl) {
  return {BracketFormula{std::move(impl)}};
}

}  // namespace fol::parser

