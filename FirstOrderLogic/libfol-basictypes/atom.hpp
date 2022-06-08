#pragma once

#include <iostream>
#include <libfol-basictypes/term.hpp>
#include <libfol-basictypes/variable.hpp>
#include <libfol-parser/parser/types.hpp>
#include <optional>
#include <string>
#include <vector>

namespace fol::types {
class Atom {
 public:
  friend std::ostream& operator<<(std::ostream& os, const Atom& atom);

  Atom(parser::PredicateFormula formula)
      : predicate_name_(formula.data.first),
        term_list_(FromTermList(std::move(formula.data.second))) {}

  Atom(parser::NotFormula formula);

  Atom(parser::FolFormula formula);

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

  bool operator==(const Atom& o) const;

  bool operator<(const Atom& o) const;

  void Substitute(const Variable& from, const Term& to);

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
