#pragma once

#include <details/utils/utility.hpp>
#include <fstream>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/exceptions.hpp>
#include <libfol-parser/parser/types.hpp>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace fol::parser {

inline UnaryFormula ParseFolFormula(lexer::LexemeGenerator::iterator &iterator);
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
          [&](lexer::Comma) -> TermListPrime {
            ++iterator;
            return {std::make_unique<TermList>(ParseTermList(iterator))};
          },
          [&](...) -> TermListPrime { return {lexer::EPS{}}; }},
      var);
}

inline TermList ParseTermList(lexer::LexemeGenerator::iterator &iterator) {
  return TermList{ParseTerm(iterator), ParseTermListPrime(iterator)};
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
            return {std::make_unique<
                std::pair<UnaryFormula, ConjuctionPrimeFormula>>(
                std::move(fol_formula), std::move(conj_prime))};
          },
          [](...) -> ConjuctionPrimeFormula { return {lexer::EPS{}}; }},
      var);
}

inline ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto fol_formula = ParseFolFormula(iterator);
  auto conj_prime = ParseConjuctionPrimeFormula(iterator);
  return {std::make_unique<std::pair<UnaryFormula, ConjuctionPrimeFormula>>(
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
  return {std::move(conj), std::move(disj_prime)};
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

inline UnaryFormula ParseFolFormula(
    lexer::LexemeGenerator::iterator &iterator) {
  auto var = *iterator;
  return {std::visit(
      details::utils::Overloaded{
          [&iterator](lexer::OpenBracket) -> UnaryFormula {
            ++iterator;
            ImplicationFormula result = ParseImplicationFormula(iterator);
            if (!std::holds_alternative<lexer::CloseBracket>(*iterator)) {
              throw ParseError{"No close bracket at (<impl>)."};
            }
            ++iterator;

            return {BracketFormula{std::move(result)}};
          },
          [&iterator](lexer::Not) -> UnaryFormula {
            ++iterator;
            return {NotFormula{
                std::make_unique<UnaryFormula>(ParseFolFormula(iterator))}};
          },
          [&iterator](lexer::Forall) -> UnaryFormula {
            ++iterator;
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
          [&iterator](lexer::Exists) -> UnaryFormula {
            ++iterator;
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
          [&iterator](lexer::Predicate predicate) -> UnaryFormula {
            ++iterator;
            if (!std::holds_alternative<lexer::OpenBracket>(*iterator)) {
              throw ParseError{"No args for predicate."};
            }
            ++iterator;

            auto pred = UnaryFormula{PredicateFormula{
                {std::move(predicate), ParseTermList(iterator)}}};
            if (!std::holds_alternative<lexer::CloseBracket>(*iterator)) {
              throw ParseError{"No close bracket at args for predicate."};
            }
            ++iterator;

            return pred;
          },
          [&](...) -> UnaryFormula {
            return UnaryFormula{ParseImplicationFormula(iterator)};
          }},
      var)};
}

inline FolFormula Parse(lexer::LexemeGenerator generator) {
  auto it = generator.begin();
  return ParseImplicationFormula(it);
}

}  // namespace fol::parser

