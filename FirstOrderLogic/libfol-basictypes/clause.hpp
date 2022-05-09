#pragma once

#include <algorithm>
#include <libfol-basictypes/atom.hpp>
#include <vector>

namespace fol::types {
class Clause {
 public:
  Clause(const std::vector<Atom>& atoms) : atoms_(atoms) {
    std::sort(atoms_.begin(), atoms_.end());
  }
  const std::vector<Atom>& atoms() const { return atoms_; }
  std::vector<Atom>& atoms() { return atoms_; }

  bool operator==(const Clause& o) const {
    return std::equal(atoms_.begin(), atoms_.end(), o.atoms_.begin());
  }

  bool empty() const { return atoms_.empty(); }

  bool IsTautology() const;

 private:
  std::vector<Atom> atoms_;
};
};  // namespace fol::types
