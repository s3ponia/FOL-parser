#pragma once

#include <libfol-parser/lexer/lexer.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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
  Term() = default;
  Term(lexer::Constant c) : data(c) {}
  Term(lexer::Variable v) : data(v) {}
  Term(FunctionFormula f) : data(std::move(f)) {}

  bool IsConstant() const { return data.index() == 0; }
  bool IsVar() const { return data.index() == 1; }
  bool IsFunction() const { return data.index() == 2; }

  const auto& Const() const { return std::get<0>(data); }
  auto& Const() { return std::get<0>(data); }
  const auto& Var() const { return std::get<1>(data); }
  auto& Var() { return std::get<1>(data); }
  const auto& Function() const { return std::get<2>(data); }
  auto& Function() { return std::get<2>(data); }

  std::variant<lexer::Constant, lexer::Variable, FunctionFormula> data;
};

namespace literals {
Term operator""_t(const char* str, std::size_t);
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
  template <typename T>
  UnaryFormula(T&& t) : data(std::forward<T>(t)) {}

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

BracketFormula MakeBrackets(parser::ImplicationFormula impl);

ExistsFormula MakeExists(std::string var, parser::ImplicationFormula impl);

ForallFormula MakeForall(std::string var, parser::ImplicationFormula impl);

NotFormula MakeNot(UnaryFormula unary);

ConjunctionFormula MakeConj(UnaryFormula unary);

ConjunctionFormula MakeConj(UnaryFormula unary, ConjunctionFormula conj);

DisjunctionFormula MakeDisj(ConjunctionFormula conj);

DisjunctionFormula MakeDisj(ConjunctionFormula conj, DisjunctionFormula disj);
ImplicationFormula MakeImpl(DisjunctionFormula disj);

ImplicationFormula MakeImpl(DisjunctionFormula disj, ImplicationFormula impl);

BracketFormula operator!(ImplicationFormula impl);

BracketFormula operator!(DisjunctionFormula impl);

NotFormula operator~(UnaryFormula impl);

ImplicationFormula operator>>=(ConjunctionFormula disj,
                               ImplicationFormula impl);

ImplicationFormula operator>>=(DisjunctionFormula disj,
                               ImplicationFormula impl);

ImplicationFormula operator>>=(UnaryFormula disj, ImplicationFormula impl);

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              DisjunctionFormula disj_rhs);

DisjunctionFormula operator||(DisjunctionFormula disj_lhs,
                              UnaryFormula unary_rhs);

DisjunctionFormula operator||(UnaryFormula conj_lhs, UnaryFormula conj_rhs);

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              UnaryFormula conj_rhs);

DisjunctionFormula operator||(UnaryFormula conj_lhs,
                              ConjunctionFormula conj_rhs);

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              ConjunctionFormula conj_rhs);

DisjunctionFormula operator||(DisjunctionFormula lhs, DisjunctionFormula rhs);

ConjunctionFormula operator&&(UnaryFormula fol_lhs, UnaryFormula fol_rhs);

ConjunctionFormula operator&&(ConjunctionFormula lhs, ConjunctionFormula rhs);

ConjunctionFormula operator&&(UnaryFormula unary_lhs,
                              ConjunctionFormula conj_rhs);

ConjunctionFormula operator&&(ConjunctionFormula conj_lhs,
                              UnaryFormula unary_rhs);

FunctionFormula operator*(lexer::Function function, TermList term_list);

TermList operator|=(Term term, TermList term_list);

UnaryFormula operator*(lexer::Predicate pred, TermList term_list);

UnaryFormula ForAll(std::string var, ImplicationFormula impl);

UnaryFormula Exists(std::string var, ImplicationFormula impl);

UnaryFormula B$(ImplicationFormula impl);

FolFormula ToFol(ImplicationFormula formula);

FolFormula ToFol(DisjunctionFormula formula);

FolFormula ToFol(ConjunctionFormula formula);

FolFormula ToFol(UnaryFormula formula);

template <class Ret>
struct TermListIterator {
  TermListIterator() = default;
  TermListIterator(TermList* term_list) : term_list_(term_list) {}

  bool operator==(const TermListIterator& o) const {
    return o.term_list_ == term_list_;
  }

  auto operator++() {
    if (term_list_.has_value()) {
      term_list_ = PopTermList(term_list_.value()).second;
    }
    return *this;
  }

  auto operator++(int) {
    auto sv = *this;
    return sv;
  }

  Ret* operator->() const { return PopTermList(term_list_.value()).first; }

  Ret& operator*() const { return *PopTermList(term_list_.value()).first; }

 private:
  static std::pair<Ret*, std::optional<parser::TermList*>> PopTermList(
      parser::TermList* term_list) {
    if (std::holds_alternative<lexer::EPS>(term_list->data.second.data)) {
      return {&(term_list->data.first), std::nullopt};
    } else {
      return {&(term_list->data.first),
              &(*std::get<std::unique_ptr<parser::TermList>>(
                  term_list->data.second.data))};
    }
  }

  std::optional<TermList*> term_list_;
};

using TermListIt = TermListIterator<Term>;
using ConstTermListIt = TermListIterator<const Term>;

std::pair<Term, std::optional<parser::TermList>> PopTermList(
    parser::TermList term_list);

std::vector<Term> FromTermList(parser::TermList term_list);

const std::string& FunctionName(const FunctionFormula& fun);

TermListIt FunctionTermsIt(FunctionFormula& fun);

std::vector<Term> FunctionTerms(FunctionFormula fun);

}  // namespace fol::parser

