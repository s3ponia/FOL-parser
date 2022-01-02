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
struct Term;
struct TermList;

struct Term {
  std::variant<lexer::Constant, lexer::Variable,
               std::unique_ptr<std::pair<lexer::Function, TermList>>>
      data;
};

struct TermList {
  std::variant<Term, std::unique_ptr<std::pair<Term, TermList>>> data;
};

struct ConjuctionPrimeFormula {
  std::variant<std::unique_ptr<std::pair<FolFormula, ConjuctionPrimeFormula>>,
               std::monostate>
      data;
};

struct ConjunctionFormula {
  std::unique_ptr<std::pair<FolFormula, ConjuctionPrimeFormula>> data;
};

struct DisjunctionPrimeFormula {
  std::variant<
      std::unique_ptr<std::pair<ConjunctionFormula, DisjunctionPrimeFormula>>,
      std::monostate>
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

inline TermList ParseTermList(lexer::LexemeGenerator generator);

inline ImplicationFormula ParseImplicationFormula(
    lexer::LexemeGenerator generator);

inline FolFormula ParseFolFormula(lexer::LexemeGenerator generator) {
  return {std::visit(
      details::utils::Overloaded{
          [&generator](lexer::OpenBracket) -> FolFormula {
            ImplicationFormula result =
                ParseImplicationFormula(std::move(generator));
            if (!std::holds_alternative<lexer::CloseBracket>(
                    details::utils::GetValueFromGenerator(generator))) {
              throw ParseError{"Error in parsing (<unary): no closebracket."};
            }
            return {BracketFormula{std::move(result)}};
          },
          [&generator](lexer::Not) -> FolFormula {
            return {NotFormula{std::make_unique<FolFormula>(
                ParseFolFormula(std::move(generator)))}};
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
            ImplicationFormula impl =
                ParseImplicationFormula(std::move(generator));
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
            ImplicationFormula impl =
                ParseImplicationFormula(std::move(generator));
            return {ForallFormula{
                {std::move(std::get<lexer::Variable>(var)), std::move(impl)}}};
          },
          [&generator](lexer::Predicate predicate) -> FolFormula {
            return {PredicateFormula{
                {std::move(predicate), ParseTermList(std::move(generator))}}};
          },
          [](auto&&) -> FolFormula {
            throw ParseError{"Unhandled fol variant"};
          }},
      std::move(details::utils::GetValueFromGenerator(generator)))};
}

}  // namespace fol::parser

