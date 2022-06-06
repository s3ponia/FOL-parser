#pragma once

#include <iostream>
#include <libfol-basictypes/term.hpp>
#include <libfol-basictypes/variable.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-transform/replace.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace fol::types {
class Atom {
 public:
  friend std::ostream& operator<<(std::ostream& os, const Atom& atom) {
    if (atom.negative_) {
      os << "~";
    }
    os << atom.predicate_name_ << "(";
    for (std::vector<Term>::size_type i = 0; i < atom.term_list_.size() - 1;
         ++i) {
      os << parser::ToString(atom.term_list_[i]) << ", ";
    }
    if (atom.term_list_.size() > 0) {
      os << parser::ToString(atom.term_list_.back());
    }
    os << ")";
    return os;
  }

  Atom(parser::PredicateFormula formula)
      : predicate_name_(formula.data.first),
        term_list_(FromTermList(std::move(formula.data.second))) {}

  Atom(parser::NotFormula formula) {
    if (matcher::check::Not(matcher::check::Pred())(formula)) {
      std::optional<parser::PredicateFormula> pred;
      Not(matcher::RefPred(pred)).match(ToFol(std::move(formula)));
      predicate_name_ = pred->data.first;
      term_list_ = FromTermList(std::move(pred->data.second));
      negative_ = true;
    } else {
      throw std::invalid_argument(
          "Atom(parser::NotFormula): formula must be negative predicate");
    }
  }

  Atom(parser::FolFormula formula) {
    if (matcher::check::Not()(formula)) {
      std::optional<parser::NotFormula> not_f;
      matcher::RefNot(not_f).match(std::move(formula));
      *this = Atom(std::move(*not_f));
      negative_ = true;
    } else {
      std::optional<parser::PredicateFormula> pred;
      matcher::RefPred(pred).match(std::move(formula));
      *this = Atom(std::move(*pred));
    }
  }

  Atom(Atom&& a) { a.swap(*this); }

  Atom(const Atom& a)
      : negative_(a.negative_), predicate_name_(a.predicate_name_) {
    term_list_.reserve(a.terms_size());
    for (auto& t : a.term_list_) {
      term_list_.push_back(Clone(t));
    }
  }

  Atom& operator=(const Atom& a) {
    auto copy = a;
    copy.swap(*this);
    return *this;
  }

  void swap(Atom& o) {
    o.term_list_.swap(term_list_);
    o.predicate_name_.swap(predicate_name_);
    std::swap(o.negative_, negative_);
  }

  bool operator==(const Atom& o) const {
    return parser::ToString(*this) == parser::ToString(o);
  }

  bool operator<(const Atom& o) const {
    return parser::ToString(*this) < parser::ToString(o);
  }

  void Substitute(const Variable& from, const Term& to) {
    for (auto& term : term_list_) {
      if (term.IsVar() && term.Var() == from) {
        term = transform::ReplaceTerm(term, from, parser::ToString(to));
      } else {
        term = transform::ReplaceTerm(term, from + ",",
                                      parser::ToString(to) + ",");
        term = transform::ReplaceTerm(term, from + ")",
                                      parser::ToString(to) + ")");
      }
    }
  }

  bool negative() const { return negative_; }
  const Term& operator[](std::size_t i) const { return term_list_[i]; }
  Term& operator[](std::size_t i) { return term_list_[i]; }
  const auto& terms() const { return term_list_; }
  auto& terms() { return term_list_; }
  std::size_t terms_size() const { return term_list_.size(); }
  const std::string& predicate_name() const { return predicate_name_; }

 private:
  bool negative_ = false;
  std::string predicate_name_;
  std::vector<Term> term_list_;
};
}  // namespace fol::types
