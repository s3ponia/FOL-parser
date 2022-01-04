#pragma once

#include <iostream>
#include <libfol-parser/parser/types.hpp>
#include <sstream>

namespace fol::parser {
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
  return os << fun_formula.data->first << "(" << fun_formula.data->second
            << ")";
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
            return os << ", " << *ptr;
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

template <class T>
inline std::string ToString(const T &formula) {
  std::ostringstream osstream;
  osstream << formula;
  return osstream.str();
}
}  // namespace fol::parser

