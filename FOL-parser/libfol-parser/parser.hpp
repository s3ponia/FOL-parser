#pragma once

#include <details/utils/utility.hpp>
#include <fstream>
#include <libfol-parser/lexer.hpp>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
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

struct TermListPrime {
  std::variant<std::unique_ptr<TermList>, lexer::EPS> data;
};

struct TermList {
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

inline std::ostream &operator<<(std::ostream &, const FolFormula &);
inline std::ostream &operator<<(std::ostream &, const TermList &);

inline std::ostream &operator<<(std::ostream &os,
                                const ConjuctionPrimeFormula &conj_prime) {
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::EPS) -> std::ostream & { return os << lexer::EPS{}; },
          [&](const std::unique_ptr<
              std::pair<FolFormula, ConjuctionPrimeFormula>> &ptr)
              -> std::ostream & {
            return os << " and " << ptr->first << ptr->second;
          }},
      conj_prime.data);
}

inline std::ostream &operator<<(std::ostream &os,
                                const ConjunctionFormula &conj) {
  return os << conj.data->first << conj.data->second;
}

inline std::ostream &operator<<(std::ostream &os,
                                const DisjunctionPrimeFormula &disj_prime) {
  return std::visit(
      details::utils::Overloaded{
          [&](const std::unique_ptr<
              std::pair<ConjunctionFormula, DisjunctionPrimeFormula>> &ptr)
              -> std::ostream & {
            return os << "or " << ptr->first << ptr->second;
          },
          [&](lexer::EPS) -> std::ostream & { return os << lexer::EPS{}; }},
      disj_prime.data);
}

inline std::ostream &operator<<(std::ostream &os,
                                const DisjunctionFormula &disj) {
  return os << disj.data.first << disj.data.second;
}

inline std::ostream &operator<<(std::ostream &os,
                                const ImplicationFormula &impl) {
  return std::visit(
      details::utils::Overloaded{
          [&](const DisjunctionFormula &formula) -> std::ostream & {
            return os << formula;
          },
          [&](const std::unique_ptr<
              std::pair<DisjunctionFormula, ImplicationFormula>> &ptr)
              -> std::ostream & {
            return os << ptr->first << "->" << ptr->second;
          }},
      impl.data);
}

inline std::ostream &operator<<(std::ostream &os,
                                const BracketFormula &br_formula) {
  return os << "(" << br_formula.data << ")";
}

inline std::ostream &operator<<(std::ostream &os,
                                const NotFormula &not_formula) {
  return os << "not " << not_formula.data;
}

inline std::ostream &operator<<(std::ostream &os,
                                const ForallFormula &forall_formula) {
  return os << "@ " << forall_formula.data.first << " . "
            << forall_formula.data.second;
}

inline std::ostream &operator<<(std::ostream &os,
                                const ExistsFormula &exists_formula) {
  return os << "? " << exists_formula.data.first << " . "
            << exists_formula.data.second;
}

inline std::ostream &operator<<(std::ostream &os,
                                const FunctionFormula &fun_formula) {
  return os << fun_formula.data->first << fun_formula.data->second;
}

inline std::ostream &operator<<(std::ostream &os, const Term &term) {
  return std::visit(
      details::utils::Overloaded{
          [&](const auto &var) -> std::ostream & { return os << var; },
      },
      term.data);
}

inline std::ostream &operator<<(std::ostream &os,
                                const TermListPrime &term_list_prime) {
  return std::visit(
      details::utils::Overloaded{
          [&](const std::unique_ptr<TermList> &ptr) -> std::ostream & {
            return os << *ptr;
          },
          [&](lexer::EPS) -> std::ostream & { return os << lexer::EPS{}; }},
      term_list_prime.data);
}

inline std::ostream &operator<<(std::ostream &os, const TermList &term_list) {
  return os << term_list.data.first << term_list.data.second;
}

inline std::ostream &operator<<(std::ostream &os,
                                const PredicateFormula &pred) {
  return os << pred.data.first << "(" << pred.data.second << ")";
}

inline std::ostream &operator<<(std::ostream &os,
                                const FolFormula &fol_formula) {
  return std::visit([&](auto &&d) -> std::ostream & { return os << d; },
                    fol_formula.data);
}

inline FolFormula ParseFolFormula(lexer::LexemeGenerator::iterator &iterator);
inline Term ParseTerm(lexer::LexemeGenerator::iterator &iterator);
inline TermList ParseTermList(lexer::LexemeGenerator::iterator &iterator);
inline DisjunctionFormula ParseDisjunctionFormula(
    lexer::LexemeGenerator::iterator &iterator);
inline DisjunctionPrimeFormula ParseDisjunctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator);
inline ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator::iterator &iterator);
inline ConjuctionPrimeFormula ParseConjuctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator);

inline Term ParseTerm(lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  ++iterator;
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::Constant constant) -> Term {
            return {std::move(constant)};
          },
          [&](lexer::Variable var) -> Term { return {std::move(var)}; },
          [&](lexer::Function func) -> Term {
            if (!std::holds_alternative<lexer::OpenBracket>(*iterator)) {
              throw ParseError{"No args for function."};
            }
            ++iterator;

            auto fun = Term{FunctionFormula{
                std::make_unique<std::pair<lexer::Function, TermList>>(
                    std::move(func), ParseTermList(iterator))}};

            if (!std::holds_alternative<lexer::CloseBracket>(*iterator)) {
              throw ParseError{"No close bracket at args for function."};
            }
            ++iterator;

            return fun;
          },
          [](...) -> Term {
            throw ParseError{"Unhandled variant in Term parsing."};
          }},
      var);
}

inline TermListPrime ParseTermListPrime(
    lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::Coma) -> TermListPrime {
            ++iterator;
            return {std::make_unique<TermList>(ParseTermList(iterator))};
          },
          [&](...) -> TermListPrime { return {lexer::EPS{}}; }},
      var);
}

inline TermList ParseTermList(lexer::LexemeGenerator::iterator &iterator) {
  return TermList{{ParseTerm(iterator), ParseTermListPrime(iterator)}};
}

inline ConjuctionPrimeFormula ParseConjuctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::And) -> ConjuctionPrimeFormula {
            ++iterator;
            auto fol_formula = ParseFolFormula(iterator);
            auto conj_prime = ParseConjuctionPrimeFormula(iterator);
            return {
                std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
                    std::move(fol_formula), std::move(conj_prime))};
          },
          [](...) -> ConjuctionPrimeFormula { return {lexer::EPS{}}; }},
      var);
}

inline ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto fol_formula = ParseFolFormula(iterator);
  auto conj_prime = ParseConjuctionPrimeFormula(iterator);
  return {std::make_unique<std::pair<FolFormula, ConjuctionPrimeFormula>>(
      std::move(fol_formula), std::move(conj_prime))};
}

inline DisjunctionPrimeFormula ParseDisjunctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  return std::visit(
      details::utils::Overloaded{
          [&](lexer::Or) -> DisjunctionPrimeFormula {
            ++iterator;
            auto conj = ParseConjuctionFormula(iterator);
            auto disj_prime = ParseDisjunctionPrimeFormula(iterator);
            return {std::make_unique<
                std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>(
                std::move(conj), std::move(disj_prime))};
          },
          [](...) -> DisjunctionPrimeFormula { return {lexer::EPS{}}; }},
      var);
}

inline DisjunctionFormula ParseDisjunctionFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto conj = ParseConjuctionFormula(iterator);
  auto disj_prime = ParseDisjunctionPrimeFormula(iterator);
  return {{std::move(conj), std::move(disj_prime)}};
}

inline ImplicationFormula ParseImplicationFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto disjunction = ParseDisjunctionFormula(iterator);

  auto next_symbol = *iterator;
  if (!std::holds_alternative<lexer::Implies>(next_symbol)) {
    return {std::move(disjunction)};
  }

  ++iterator;

  auto impl = ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disjunction), ParseImplicationFormula(iterator))};
  return impl;
}

inline FolFormula ParseFolFormula(lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  ++iterator;
  return {std::visit(
      details::utils::Overloaded{
          [&iterator](lexer::OpenBracket) -> FolFormula {
            ImplicationFormula result = ParseImplicationFormula(iterator);
            if (!std::holds_alternative<lexer::CloseBracket>(*iterator)) {
              throw ParseError{"No close bracket at (<impl>)."};
            }
            ++iterator;

            return {BracketFormula{std::move(result)}};
          },
          [&iterator](lexer::Not) -> FolFormula {
            return {NotFormula{
                std::make_unique<FolFormula>(ParseFolFormula(iterator))}};
          },
          [&iterator](lexer::Forall) -> FolFormula {
            auto var = *iterator;
            if (!std::holds_alternative<lexer::Variable>(var)) {
              throw ParseError{
                  "Error in parsing @ <var> . <impl>: no variable after @."};
            }
            ++iterator;

            if (!std::holds_alternative<lexer::Dot>(*iterator)) {
              throw ParseError{
                  "Error in parsing @ <var> . <impl>: no . after <var>."};
            }
            ++iterator;

            ImplicationFormula impl = ParseImplicationFormula(iterator);
            return {ForallFormula{
                {std::move(std::get<lexer::Variable>(var)), std::move(impl)}}};
          },
          [&iterator](lexer::Exists) -> FolFormula {
            auto var = *iterator;
            if (!std::holds_alternative<lexer::Variable>(var)) {
              throw ParseError{
                  "Error in parsing ? <var> . <impl>: no variable after @."};
            }
            ++iterator;

            if (!std::holds_alternative<lexer::Dot>(*iterator)) {
              throw ParseError{
                  "Error in parsing ? <var> . <impl>: no . after <var>."};
            }
            ++iterator;

            ImplicationFormula impl = ParseImplicationFormula(iterator);
            return {ForallFormula{
                {std::move(std::get<lexer::Variable>(var)), std::move(impl)}}};
          },
          [&iterator](lexer::Predicate predicate) -> FolFormula {
            if (!std::holds_alternative<lexer::OpenBracket>(*iterator)) {
              throw ParseError{"No args for predicate."};
            }
            ++iterator;

            auto pred = FolFormula{PredicateFormula{
                {std::move(predicate), ParseTermList(iterator)}}};
            if (!std::holds_alternative<lexer::CloseBracket>(*iterator)) {
              throw ParseError{"No close bracket at args for predicate."};
            }
            ++iterator;

            return pred;
          },
          [&](...) -> FolFormula {
            throw ParseError{"Unhandled variant in FolFormula parsing."};
          }},
      var)};
}

inline FolFormula Parse(lexer::LexemeGenerator iterator) {
  auto it = iterator.begin();
  return ParseFolFormula(it);
}

inline ImplicationFormula operator->*(DisjunctionFormula disj,
                                      ImplicationFormula impl) {
  return ImplicationFormula{
      std::make_unique<std::pair<DisjunctionFormula, ImplicationFormula>>(
          std::move(disj), std::move(impl))};
}

/*
inline DisjunctionFormula operator||(ConjunctionFormula conj_lhs,
                                     ConjunctionFormula conj_rhs) {
  return DisjunctionFormula{{std::move(conj_lhs), DisjunctionPrimeFormula{}}};
}
*/

}  // namespace fol::parser

