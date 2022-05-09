#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <optional>
#include <utility>
#include <vector>

namespace fol::types {
class IClausesStorage {
 public:
  virtual std::optional<Clause> NextClause() = 0;
  virtual void AddClause(const Clause&) = 0;
  virtual bool empty() const = 0;
  // true if passed clause is subsumed by set
  virtual bool Simplify(const Clause&) = 0;
  virtual std::vector<Clause> Infer(const Clause&,
                                    const unification::IUnificator&) const = 0;
};
}  // namespace fol::types
