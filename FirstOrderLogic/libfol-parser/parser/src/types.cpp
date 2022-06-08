#include <libfol-parser/parser/types.hpp>

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

namespace literals {
Term operator""_t(const char* str, std::size_t) {
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

BracketFormula MakeBrackets(parser::ImplicationFormula impl) {
  return BracketFormula{std::move(impl)};
}

ExistsFormula MakeExists(std::string var, parser::ImplicationFormula impl) {
  return {{std::move(var), std::move(impl)}};
}

ForallFormula MakeForall(std::string var, parser::ImplicationFormula impl) {
  return {{std::move(var), std::move(impl)}};
}

NotFormula MakeNot(UnaryFormula unary) {
  return {std::make_unique<UnaryFormula>(std::move(unary))};
}

ConjunctionFormula MakeConj(UnaryFormula unary) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(unary), ConjunctionPrimeFormula{lexer::EPS{}})};
}

ConjunctionFormula MakeConj(UnaryFormula unary, ConjunctionFormula conj) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(unary), ConjunctionPrimeFormula{std::move(conj.data)})};
}

DisjunctionFormula MakeDisj(ConjunctionFormula conj) {
  return {std::move(conj), DisjunctionPrimeFormula{lexer::EPS{}}};
}

DisjunctionFormula MakeDisj(ConjunctionFormula conj, DisjunctionFormula disj) {
  return {std::move(conj),
          DisjunctionPrimeFormula{std::make_unique<
              std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
              std::move(disj.data))}};
}
ImplicationFormula MakeImpl(DisjunctionFormula disj) {
  return {std::move(disj)};
}

ImplicationFormula MakeImpl(DisjunctionFormula disj, ImplicationFormula impl) {
  return {std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
      std::move(disj), std::move(impl))};
}

BracketFormula operator!(ImplicationFormula impl) {
  return parser::MakeBrackets(std::move(impl));
}

BracketFormula operator!(DisjunctionFormula impl) {
  return parser::MakeBrackets({std::move(impl)});
}

NotFormula operator~(UnaryFormula impl) {
  return parser::MakeNot(std::move(impl));
}

ImplicationFormula operator>>=(ConjunctionFormula disj,
                               ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

ImplicationFormula operator>>=(DisjunctionFormula disj,
                               ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

ImplicationFormula operator>>=(UnaryFormula disj, ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              DisjunctionFormula disj_rhs) {
  return MakeDisj(std::move(conj_lhs), std::move(disj_rhs));
}

DisjunctionFormula operator||(DisjunctionFormula disj_lhs,
                              UnaryFormula unary_rhs) {
  return MakeDisj(
      std::move(disj_lhs.data.first),
      {MakeConj(std::move(unary_rhs)), std::move(disj_lhs.data.second)});
}

DisjunctionFormula operator||(UnaryFormula conj_lhs, UnaryFormula conj_rhs) {
  return DisjunctionFormula{
      MakeConj(std::move(conj_lhs)),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          MakeConj(std::move(conj_rhs)),
          DisjunctionPrimeFormula{lexer::EPS{}})}};
}

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              UnaryFormula conj_rhs) {
  return DisjunctionFormula{
      std::move(conj_lhs),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          MakeConj(std::move(conj_rhs)),
          DisjunctionPrimeFormula{lexer::EPS{}})}};
}

DisjunctionFormula operator||(UnaryFormula conj_lhs,
                              ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{
      MakeConj(std::move(conj_lhs)),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(conj_rhs), DisjunctionPrimeFormula{lexer::EPS{}})}};
}

DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                              ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{
      std::move(conj_lhs),
      DisjunctionPrimeFormula{std::make_unique<
          std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
          std::move(conj_rhs), DisjunctionPrimeFormula{lexer::EPS{}})}};
}

DisjunctionFormula operator||(DisjunctionFormula lhs, DisjunctionFormula rhs) {
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

ConjunctionFormula operator&&(UnaryFormula fol_lhs, UnaryFormula fol_rhs) {
  return {std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
      std::move(fol_lhs),
      ConjunctionPrimeFormula{
          std::make_unique<std::pair<UnaryFormula, ConjunctionPrimeFormula>>(
              std::move(fol_rhs), ConjunctionPrimeFormula{lexer::EPS{}})})};
}

ConjunctionFormula operator&&(ConjunctionFormula lhs, ConjunctionFormula rhs) {
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

ConjunctionFormula operator&&(UnaryFormula unary_lhs,
                              ConjunctionFormula conj_rhs) {
  return MakeConj(std::move(unary_lhs), std::move(conj_rhs));
}

ConjunctionFormula operator&&(ConjunctionFormula conj_lhs,
                              UnaryFormula unary_rhs) {
  return std::move(unary_rhs) && std::move(conj_lhs);
}

FunctionFormula operator*(lexer::Function function, TermList term_list) {
  return {std::make_unique<std::pair<lexer::Function, TermList>>(
      std::move(function), std::move(term_list))};
}

TermList operator|=(Term term, TermList term_list) {
  return TermList{
      std::move(term),
      TermListPrime{std::make_unique<TermList>(std::move(term_list))}};
}

UnaryFormula operator*(lexer::Predicate pred, TermList term_list) {
  return {PredicateFormula{{std::move(pred), std::move(term_list)}}};
}

UnaryFormula ForAll(std::string var, ImplicationFormula impl) {
  return {ForallFormula{{std::move(var), std::move(impl)}}};
}

UnaryFormula Exists(std::string var, ImplicationFormula impl) {
  return {ExistsFormula{{std::move(var), std::move(impl)}}};
}

UnaryFormula B$(ImplicationFormula impl) {
  return {BracketFormula{std::move(impl)}};
}

FolFormula ToFol(ImplicationFormula formula) { return formula; }

FolFormula ToFol(DisjunctionFormula formula) { return {std::move(formula)}; }

FolFormula ToFol(ConjunctionFormula formula) {
  return ToFol(MakeDisj(std::move(formula)));
}

FolFormula ToFol(UnaryFormula formula) {
  return ToFol(MakeConj(std::move(formula)));
}

std::pair<Term, std::optional<parser::TermList>> PopTermList(
    parser::TermList term_list) {
  if (std::holds_alternative<lexer::EPS>(term_list.data.second.data)) {
    return {std::move(term_list.data.first), std::nullopt};
  } else {
    return {std::move(term_list.data.first),
            std::move(*std::get<std::unique_ptr<parser::TermList>>(
                term_list.data.second.data))};
  }
}

std::vector<Term> FromTermList(parser::TermList term_list) {
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

const std::string& FunctionName(const FunctionFormula& fun) {
  return fun.data->first;
}

TermListIt FunctionTermsIt(FunctionFormula& fun) {
  return TermListIt(&fun.data->second);
}

std::vector<Term> FunctionTerms(FunctionFormula fun) {
  return FromTermList(std::move(fun.data->second));
}

}  // namespace fol::parser

