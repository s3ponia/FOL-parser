#pragma once

#include <algorithm>
#include <libfol-basictypes/atom.hpp>
#include <vector>

namespace fol::types {
class Clause {
 public:
  friend std::ostream& operator<<(std::ostream& os, const Clause& c);

  Clause() = default;

  Clause(const std::vector<Atom>& atoms) : atoms_(atoms) {
    std::sort(atoms_.begin(), atoms_.end());
  }
  Clause(parser::FolFormula disj);

  const std::vector<Atom>& atoms() const { return atoms_; }
  std::vector<Atom>& atoms() { return atoms_; }

  void EraseAtom(std::size_t id) { atoms_.erase(atoms_.cbegin() + id); }

  Clause& operator+=(const Clause& o);

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
  static inline std::size_t counter = 0;

  std::vector<Atom> atoms_;
  std::vector<Clause> ancestors_;
  std::size_t id_ = ++counter;
};
};  // namespace fol::types
