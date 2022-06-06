#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/term.hpp>
#include <libfol-basictypes/variable.hpp>
#include <ostream>
#include <vector>

namespace fol::unification {
class Substitution {
 public:
  struct SubstitutePair {
    types::Variable from;
    types::Term to;

    SubstitutePair(const SubstitutePair& o)
        : from(o.from), to(types::Clone(o.to)) {}

    SubstitutePair(const types::Variable& from, const types::Term& to)
        : from(from), to(types::Clone(to)) {}
  };

  friend std::ostream& operator<<(std::ostream& os, const Substitution& sub) {
    os << "[ ";
    for (auto& p : sub.substitute_pairs_) {
      os << "{" << p.to << "/" << p.from << "} ";
    }
    os << "]";
    return os;
  }

  Substitution() = default;

  Substitution(const std::vector<SubstitutePair>& substitution)
      : substitute_pairs_(substitution) {}

  Substitution& operator+=(const Substitution& o) {
    for (auto& s_p : substitute_pairs_) {
      o.Substitute(s_p.to);
    }

    substitute_pairs_.reserve(substitute_pairs_.size() +
                              o.substitute_pairs_.size());

    for (auto& o_s_p : o.substitute_pairs_) {
      if (!std::any_of(substitute_pairs_.begin(), substitute_pairs_.end(),
                       [&](auto&& s_p) { return s_p.from == o_s_p.from; })) {
        substitute_pairs_.push_back(o_s_p);
      }
    }

    FilterUselessPairs();

    return *this;
  }

  void Substitute(types::Term& term) const {
    for (auto&& sub_pair : substitute_pairs_) {
      auto&& from = sub_pair.from;
      auto&& to = sub_pair.to;
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

  void Substitute(types::Atom& atom) const {
    for (auto&& sub_pair : substitute_pairs_) {
      atom.Substitute(sub_pair.from, sub_pair.to);
    }
  }

  void Substitute(types::Clause& clause) const {
    for (auto& atom : clause.atoms()) {
      for (auto&& sub_pair : substitute_pairs_) {
        atom.Substitute(sub_pair.from, sub_pair.to);
      }
    }
  }

 private:
  void FilterUselessPairs() {
    std::vector<SubstitutePair> res;
    res.reserve(substitute_pairs_.size());
    for (auto& s_p : substitute_pairs_) {
      if (s_p.from != parser::ToString(s_p.to)) {
        res.push_back(s_p);
      }
    }

    substitute_pairs_ = std::move(res);
  }

  std::vector<SubstitutePair> substitute_pairs_;
};
}  // namespace fol::unification
