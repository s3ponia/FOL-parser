#include <libfol-basictypes/atom.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/replace.hpp>
#include <stdexcept>

namespace fol::types {
std::ostream& operator<<(std::ostream& os, const Atom& atom) {
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

Atom::Atom(parser::NotFormula formula) {
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

Atom::Atom(parser::FolFormula formula) {
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

void Atom::Substitute(const Variable& from, const Term& to) {
  for (auto& term : term_list_) {
    if (term.IsVar() && term.Var() == from) {
      term = transform::ReplaceTerm(term, from, parser::ToString(to));
    } else {
      term =
          transform::ReplaceTerm(term, from + ",", parser::ToString(to) + ",");
      term =
          transform::ReplaceTerm(term, from + ")", parser::ToString(to) + ")");
    }
  }
}

bool Atom::operator==(const Atom& o) const {
  return parser::ToString(*this) == parser::ToString(o);
}

bool Atom::operator<(const Atom& o) const {
  return parser::ToString(*this) < parser::ToString(o);
}
}  // namespace fol::types
