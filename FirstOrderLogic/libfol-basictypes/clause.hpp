#pragma once

#include <algorithm>
#include <cstring>
#include <details/utils/utility.hpp>
#include <libfol-basictypes/atom.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/normalization.hpp>
#include <vector>

namespace fol::types {
class Clause {
 public:
  friend std::ostream& operator<<(std::ostream& os, const Clause& c) {
    if (c.atoms_.empty()) {
      os << "EMPTY";
      return os;
    }
    for (std::size_t i = 0; i < c.atoms_.size(); ++i) {
      os << c.atoms_[i] << (i == c.atoms_.size() - 1 ? "" : " or ");
    }
    return os;
  }

  Clause() = default;

  Clause(const std::vector<Atom>& atoms) : atoms_(atoms) {
    std::sort(atoms_.begin(), atoms_.end());
  }
  Clause(parser::FolFormula disj)
      : atoms_(DisjunctionFormulaToAtoms(std::move(disj))) {}

  const std::vector<Atom>& atoms() const { return atoms_; }
  std::vector<Atom>& atoms() { return atoms_; }

  void EraseAtom(std::size_t id) { atoms_.erase(atoms_.cbegin() + id); }

  Clause& operator+=(const Clause& o) {
    atoms_.reserve(atoms_.size() + o.atoms_.size());
    atoms_.insert(atoms_.cend(), o.atoms_.begin(), o.atoms_.end());
    std::sort(atoms_.begin(), atoms_.end());
    return *this;
  }

  bool operator==(const Clause& o) const {
    return o.atoms().size() == atoms().size() &&
           std::equal(atoms_.begin(), atoms_.end(), o.atoms_.begin());
  }

  bool operator<(const Clause& o) const {
    return std::lexicographical_compare(atoms_.begin(), atoms_.end(),
                                        o.atoms_.begin(), o.atoms_.end());
  }

  void ClearAncestors() { ancestors_.clear(); }

  void GenerateId() { id_ = ++counter; }

  void AddAncestor(const Clause& o) { ancestors_.push_back(o); }

  const std::vector<Clause>& ancestors() const { return ancestors_; }

  std::size_t id() const { return id_; }

  bool empty() const { return atoms_.empty(); }

 private:
  static std::vector<Atom> DisjunctionFormulaToAtoms(parser::FolFormula disj) {
    disj = transform::DeleteUselessBrackets(std::move(disj));
    auto str = parser::ToString(disj);

    auto atoms_str = details::utils::Split(str, " or ");

    std::vector<Atom> atoms;
    atoms.reserve(atoms_str.size());

    for (auto& s : atoms_str) {
      atoms.push_back(Atom(parser::Parse(lexer::Tokenize(s))));
    }

    std::sort(atoms.begin(), atoms.end());

    return atoms;
  }

  static inline std::size_t counter = 0;

  std::vector<Atom> atoms_;
  std::vector<Clause> ancestors_;
  std::size_t id_ = ++counter;
};
};  // namespace fol::types
