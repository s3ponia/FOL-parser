#pragma once

#include <iostream>
#include <libfol-parser/parser/types.hpp>
#include <sstream>

namespace fol::parser {
std::ostream &operator<<(std::ostream &, const UnaryFormula &);
std::ostream &operator<<(std::ostream &, const TermList &);

std::ostream &operator<<(std::ostream &os,
                         const ConjunctionPrimeFormula &conj_prime);

std::ostream &operator<<(std::ostream &os, const ConjunctionFormula &conj);

std::ostream &operator<<(std::ostream &os,
                         const DisjunctionPrimeFormula &disj_prime);

std::ostream &operator<<(std::ostream &os, const DisjunctionFormula &disj);

std::ostream &operator<<(std::ostream &os, const ImplicationFormula &impl);

std::ostream &operator<<(std::ostream &os, const BracketFormula &br_formula);

std::ostream &operator<<(std::ostream &os, const NotFormula &not_formula);

std::ostream &operator<<(std::ostream &os, const ForallFormula &forall_formula);

std::ostream &operator<<(std::ostream &os, const ExistsFormula &exists_formula);

std::ostream &operator<<(std::ostream &os, const FunctionFormula &fun_formula);

std::ostream &operator<<(std::ostream &os, const Term &term);

std::ostream &operator<<(std::ostream &os,
                         const TermListPrime &term_list_prime);

std::ostream &operator<<(std::ostream &os, const TermList &term_list);

std::ostream &operator<<(std::ostream &os, const PredicateFormula &pred);

std::ostream &operator<<(std::ostream &os, const UnaryFormula &fol_formula);

template <class T>
std::string ToString(const T &formula) {
  std::ostringstream osstream;
  osstream << formula;
  return osstream.str();
}
}  // namespace fol::parser

