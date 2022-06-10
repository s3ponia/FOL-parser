#pragma once

#include <libfol-basictypes/atom.hpp>
#include <libfol-basictypes/clause.hpp>
#include <libfol-unification/substitution.hpp>
#include <optional>

namespace fol::unification {
class IUnificator {
 public:
  virtual std::optional<Substitution> Unificate(const types::Atom&,
                                                const types::Atom&) const = 0;

  void Simplify(types::Clause& clause) const;

  bool IsPartOf(const types::Clause& lhs, const types::Clause& rhs) const;

  std::optional<types::Clause> Resolution(types::Clause lhs,
                                          types::Clause rhs) const;

  bool IsTautology(const types::Clause& c) const;
};
}  // namespace fol::unification
