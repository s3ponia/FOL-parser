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
struct ConjuctionPrimeFormula;
struct FolFormula;
struct PredicateFormula;
struct FunctionFormula;
struct Term;
struct TermList;

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

struct ConjuctionPrimeFormula {
  std::variant<std::unique_ptr<std::pair<FolFormula, ConjuctionPrimeFormula>>,
               lexer::EPS>
      data;
};

struct ConjunctionFormula {
  std::unique_ptr<std::pair<FolFormula, ConjuctionPrimeFormula>> data;
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
  std::unique_ptr<FolFormula> data;
};

struct ForallFormula {
  std::pair<lexer::Variable, ImplicationFormula> data;
};

struct ExistsFormula {
  std::pair<lexer::Variable, ImplicationFormula> data;
};

struct PredicateFormula {
  std::pair<lexer::Predicate, TermList> data;
};

struct FolFormula {
  operator DisjunctionFormula() && {
    return DisjunctionFormula{ConjunctionFormula{
        std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
            std::move(*this), ConjuctionPrimeFormula{lexer::EPS{}})}};
  }
  operator ImplicationFormula() && {
    return ImplicationFormula{
        static_cast<DisjunctionFormula>(std::move(*this))};
  }
  std::variant<BracketFormula, NotFormula, ForallFormula, ExistsFormula,
               PredicateFormula, ImplicationFormula>
      data;
};

inline ImplicationFormula operator>>=(FolFormula disj,
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

inline ConjunctionFormula operator&&(FolFormula fol_lhs, FolFormula fol_rhs) {
  return {std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
      std::move(fol_lhs),
      ConjuctionPrimeFormula{
          std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
              std::move(fol_rhs), ConjuctionPrimeFormula{lexer::EPS{}})})};
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

inline FolFormula operator*(lexer::Predicate pred, TermList term_list) {
  return {PredicateFormula{{std::move(pred), std::move(term_list)}}};
}

inline FolFormula ForAll(lexer::Variable var, ImplicationFormula impl) {
  return {ForallFormula{{std::move(var), std::move(impl)}}};
}

inline FolFormula Exists(lexer::Variable var, ImplicationFormula impl) {
  return {ExistsFormula{{std::move(var), std::move(impl)}}};
}

inline FolFormula B$(ImplicationFormula impl) {
  return {BracketFormula{std::move(impl)}};
}

}  // namespace fol::parser

