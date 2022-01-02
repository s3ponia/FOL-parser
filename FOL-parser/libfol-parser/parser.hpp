#pragma once

#include <details/utils/utility.hpp>
#include <libfol-parser/lexer.hpp>
#include <memory>
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
  std::unique_ptr<std::pair<ConjunctionFormula, DisjunctionFormula>> data;
};

struct ImplicationFormula {
  std::variant<
      std::unique_ptr<DisjunctionFormula>,
      std::unique_ptr<std::pair<DisjunctionFormula, ImplicationFormula>>>
      data;
};

struct BracketFormula {
  std::unique_ptr<ImplicationFormula> data;
};

struct NotFormula {
  std::unique_ptr<FolFormula> data;
};

struct ForallFormula {
  std::unique_ptr<std::pair<lexer::Variable, ImplicationFormula>> data;
};

struct ExistsFormula {
  std::unique_ptr<std::pair<lexer::Variable, ImplicationFormula>> data;
};

struct PredicateFormula {
  std::unique_ptr<std::pair<lexer::Predicate, TermList>> data;
};

struct FolFormula {
  std::variant<BracketFormula, NotFormula, ForallFormula, ExistsFormula,
               PredicateFormula>
      data;
};

}  // namespace fol::parser

