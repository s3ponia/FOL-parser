#pragma once

#include <details/utils/utility.hpp>
#include <libfol-parser/lexer.hpp>
#include <memory>
#include <stdexcept>
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
  std::variant<lexer::Constant, lexer::Variable, FunctionFormula> data;
};

struct TermList {
  std::variant<Term, std::unique_ptr<std::pair<Term, TermList>>> data;
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
  std::variant<BracketFormula, NotFormula, ForallFormula, ExistsFormula,
               PredicateFormula>
      data;
};

struct ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline FolFormula ParseFolFormula(lexer::LexemeGenerator &generator);
inline Term ParseTerm(lexer::LexemeGenerator &generator);
inline TermList ParseTermList(lexer::LexemeGenerator &generator);
inline DisjunctionFormula ParseDisjunctionFormula(
    lexer::LexemeGenerator &generator);
inline DisjunctionPrimeFormula ParseDisjunctionPrimeFormula(
    lexer::LexemeGenerator &generator);
inline ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator &generator);
inline ConjuctionPrimeFormula ParseConjuctionPrimeFormula(
    lexer::LexemeGenerator &generator);

inline Term ParseTerm(lexer::LexemeGenerator &generator) {
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::Constant constant) -> Term {
            return {std::move(constant)};
          },
          [&](lexer::Variable var) -> Term { return {std::move(var)}; },
          [&](lexer::Function func) -> Term {
            return {FunctionFormula{
                std::make_unique<std::pair<lexer::Function, TermList>>(
                    std::move(func), ParseTermList(generator))}};
          },
          [](auto &&) -> Term {
            throw ParseError{"Unhandled variant in Term parsing."};
          }},
      details::utils::GetValueFromGenerator(generator));
}

inline TermList ParseTermList(lexer::LexemeGenerator &generator) {
  return TermList{std::make_unique<std::pair<Term, TermList>>(
      ParseTerm(generator), ParseTermList(generator))};
}

inline ConjuctionPrimeFormula ParseConjuctionPrimeFormula(
    lexer::LexemeGenerator &generator) {
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::And) -> ConjuctionPrimeFormula {
            auto fol_formula = ParseFolFormula(generator);
            auto conj_prime = ParseConjuctionPrimeFormula(generator);
            return {
                std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
                    std::move(fol_formula), std::move(conj_prime))};
          },
          [](lexer::EPS) -> ConjuctionPrimeFormula { return {lexer::EPS{}}; },
          [](auto &&) -> ConjuctionPrimeFormula {
            throw ParseError{"Unhandled variant in conjuction prime parsing."};
          }},
      details::utils::GetValueFromGenerator(generator));
}

inline ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator &generator) {
  auto fol_formula = ParseFolFormula(generator);
  auto conj_prime = ParseConjuctionPrimeFormula(generator);
  return {std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
      std::move(fol_formula), std::move(conj_prime))};
}

inline DisjunctionPrimeFormula ParseDisjunctionPrimeFormula(
    lexer::LexemeGenerator &generator) {
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::Or) -> DisjunctionPrimeFormula {
            auto conj = ParseConjuctionFormula(generator);
            auto disj_prime = ParseDisjunctionPrimeFormula(generator);
            return {std::make_unique<
                std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
                std::move(conj), std::move(disj_prime))};
          },
          [](lexer::EPS) -> DisjunctionPrimeFormula { return {lexer::EPS{}}; },
          [](auto &&) -> DisjunctionPrimeFormula {
            throw ParseError{
                "Unhandled variant in DisjunctionPrimeFormula parsing."};
          }},
      details::utils::GetValueFromGenerator(generator));
}

inline DisjunctionFormula ParseDisjunctionFormula(
    lexer::LexemeGenerator &generator) {
  auto conj = ParseConjuctionFormula(generator);
  auto disj_prime = ParseDisjunctionPrimeFormula(generator);
  return {{std::move(conj), std::move(disj_prime)}};
}

inline ImplicationFormula ParseImplicationFormula(
    lexer::LexemeGenerator &generator) {
  auto disjunction = ParseDisjunctionFormula(generator);
  if (!std::holds_alternative<lexer::Implies>(
          details::utils::GetValueFromGenerator(generator))) {
    if (!std::holds_alternative<lexer::CloseBracket>(
            details::utils::GetValueFromGenerator(generator))) {
      throw ParseError{
          "Logic error in implication parsing: If no implies sign, then it "
          "must be close bracket."};
    }

    return {std::move(disjunction)};
  }

  return {std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
      std::move(disjunction), ParseImplicationFormula(generator))};
}

inline FolFormula ParseFolFormula(lexer::LexemeGenerator &generator) {
  return {std::visit(
      details::utils::Overloaded{
          [&generator](lexer::OpenBracket) -> FolFormula {
            ImplicationFormula result = ParseImplicationFormula(generator);
            return {BracketFormula{std::move(result)}};
          },
          [&generator](lexer::Not) -> FolFormula {
            return {NotFormula{
                std::make_unique<FolFormula>(ParseFolFormula(generator))}};
          },
          [&generator](lexer::Forall) -> FolFormula {
            auto var = details::utils::GetValueFromGenerator(generator);
            if (!std::holds_alternative<lexer::Variable>(var)) {
              throw ParseError{
                  "Error in parsing @ <var> . <impl>: no variable after @."};
            }
            if (!std::holds_alternative<lexer::Dot>(
                    details::utils::GetValueFromGenerator(generator))) {
              throw ParseError{
                  "Error in parsing @ <var> . <impl>: no . after <var>."};
            }
            ImplicationFormula impl = ParseImplicationFormula(generator);
            return {ForallFormula{
                {std::move(std::get<lexer::Variable>(var)), std::move(impl)}}};
          },
          [&generator](lexer::Exists) -> FolFormula {
            auto var = details::utils::GetValueFromGenerator(generator);
            if (!std::holds_alternative<lexer::Variable>(var)) {
              throw ParseError{
                  "Error in parsing ? <var> . <impl>: no variable after @."};
            }
            if (!std::holds_alternative<lexer::Dot>(
                    details::utils::GetValueFromGenerator(generator))) {
              throw ParseError{
                  "Error in parsing ? <var> . <impl>: no . after <var>."};
            }
            ImplicationFormula impl = ParseImplicationFormula(generator);
            return {ForallFormula{
                {std::move(std::get<lexer::Variable>(var)), std::move(impl)}}};
          },
          [&generator](lexer::Predicate predicate) -> FolFormula {
            return {PredicateFormula{
                {std::move(predicate), ParseTermList(generator)}}};
          },
          [](auto &&) -> FolFormula {
            throw ParseError{"Unhandled fol variant"};
          }},
      details::utils::GetValueFromGenerator(generator))};
}

}  // namespace fol::parser

