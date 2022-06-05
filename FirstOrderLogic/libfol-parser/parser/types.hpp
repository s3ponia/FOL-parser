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

inline BracketFormula MakeBrackets(parser::ImplicationFormula impl) {
  return BracketFormula{std::move(impl)};
}

inline ExistsFormula MakeExists(std::string var,
                                parser::ImplicationFormula impl) {
  return {{std::move(var), std::move(impl)}};
}

inline ForallFormula MakeForall(std::string var,
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

inline BracketFormula operator!(ImplicationFormula impl) {
  return parser::MakeBrackets(std::move(impl));
}

inline BracketFormula operator!(DisjunctionFormula impl) {
  return parser::MakeBrackets({std::move(impl)});
}

inline NotFormula operator~(UnaryFormula impl) {
  return parser::MakeNot(std::move(impl));
}

inline ImplicationFormula operator>>=(ConjunctionFormula disj,
                                      ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

inline ImplicationFormula operator>>=(DisjunctionFormula disj,
                                      ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

inline ImplicationFormula operator>>=(UnaryFormula disj,
                                      ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

inline DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                                     DisjunctionFormula disj_rhs) {
  return MakeDisj(std::move(conj_lhs), std::move(disj_rhs));
}

inline DisjunctionFormula operator||(DisjunctionFormula disj_lhs,
                                     UnaryFormula unary_rhs) {
  return MakeDisj(
      std::move(disj_lhs.data.first),
      {MakeConj(std::move(unary_rhs)), std::move(disj_lhs.data.second)});
}

inline DisjunctionFormula operator||(UnaryFormula conj_lhs,
                                     UnaryFormula conj_rhs) {
  return DisjunctionFormula{
      MakeConj(std::move(conj_lhs)),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          MakeConj(std::move(conj_rhs)),
          DisjunctionPrimeFormula{lexer::EPS{}})}};
}

inline DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                                     UnaryFormula conj_rhs) {
  return DisjunctionFormula{
      std::move(conj_lhs),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          MakeConj(std::move(conj_rhs)),
          DisjunctionPrimeFormula{lexer::EPS{}})}};
}

inline DisjunctionFormula operator||(UnaryFormula conj_lhs,
                                     ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{
      MakeConj(std::move(conj_lhs)),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(conj_rhs), DisjunctionPrimeFormula{lexer::EPS{}})}};
}

inline DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                                     ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{
      std::move(conj_lhs),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(conj_rhs), DisjunctionPrimeFormula{lexer::EPS{}})}};
}

inline DisjunctionFormula operator||(DisjunctionFormula lhs,
                                     DisjunctionFormula rhs) {
  DisjunctionPrimeFormula* most_right_lhs = &lhs.data.second;
  while (most_right_lhs->data.index() == 0) {
    most_right_lhs = &std::get<std::unique_ptr<
        std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>>(
                          most_right_lhs->data)
                          ->second;
  }
  most_right_lhs->data =
      std::make_unique<std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(rhs.data));
  return lhs;
}

inline ConjunctionFormula operator&&(UnaryFormula fol_lhs,
                                     UnaryFormula fol_rhs) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(fol_lhs),
      ConjunctionPrimeFormula{
          std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
              std::move(fol_rhs), ConjunctionPrimeFormula{lexer::EPS{}})})};
}

inline ConjunctionFormula operator&&(ConjunctionFormula lhs,
                                     ConjunctionFormula rhs) {
  ConjunctionPrimeFormula* most_right_lhs = &lhs.data->second;
  while (most_right_lhs->data.index() == 0) {
    most_right_lhs =
        &std::get<
             std::unique_ptr<std::pair<UnaryFormula, ConjunctionPrimeFormula>>>(
             most_right_lhs->data)
             ->second;
  }
  most_right_lhs->data = std::move(rhs.data);
  return lhs;
}

inline ConjunctionFormula operator&&(UnaryFormula unary_lhs,
                                     ConjunctionFormula conj_rhs) {
  return MakeConj(std::move(unary_lhs), std::move(conj_rhs));
}

inline ConjunctionFormula operator&&(ConjunctionFormula conj_lhs,
                                     UnaryFormula unary_rhs) {
  return std::move(unary_rhs) && std::move(conj_lhs);
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

inline UnaryFormula ForAll(std::string var, ImplicationFormula impl) {
  return {ForallFormula{{std::move(var), std::move(impl)}}};
}

inline UnaryFormula Exists(std::string var, ImplicationFormula impl) {
  return {ExistsFormula{{std::move(var), std::move(impl)}}};
}

inline UnaryFormula B$(ImplicationFormula impl) {
  return {BracketFormula{std::move(impl)}};
}

inline FolFormula ToFol(ImplicationFormula formula) { return formula; }

inline FolFormula ToFol(DisjunctionFormula formula) {
  return {std::move(formula)};
}

inline FolFormula ToFol(ConjunctionFormula formula) {
  return ToFol(MakeDisj(std::move(formula)));
}

inline FolFormula ToFol(UnaryFormula formula) {
  return ToFol(MakeConj(std::move(formula)));
}

inline std::pair<Term, std::optional<parser::TermList>> PopTermList(
    parser::TermList term_list) {
  if (std::holds_alternative<lexer::EPS>(term_list.data.second.data)) {
    return {std::move(term_list.data.first), std::nullopt};
  } else {
    return {std::move(term_list.data.first),
            std::move(*std::get<std::unique_ptr<parser::TermList>>(
                term_list.data.second.data))};
  }
}

inline std::vector<Term> FromTermList(parser::TermList term_list) {
  std::vector<Term> terms;
  while (true) {
    auto [t, opt_t_list] = PopTermList(std::move(term_list));
    terms.push_back(std::move(t));

    if (opt_t_list.has_value()) {
      term_list = std::move(*opt_t_list);
    } else {
      break;
    }
  }

  return terms;
}

inline const auto& FunctionName(const FunctionFormula& fun) {
  return fun.data->first;
}

inline auto FunctionTerms(FunctionFormula fun) {
  return FromTermList(std::move(fun.data->second));
}

}  // namespace fol::parser

