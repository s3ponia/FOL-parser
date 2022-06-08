#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "libfol-unification/unification_interface.hpp"

namespace fol::types {
class StrikeoutClausesStorage : public IClausesStorage {
 public:
  using StorageType = std::list<Clause>;
  StrikeoutClausesStorage(std::unique_ptr<unification::IUnificator> unifier)
      : unifier_(std::move(unifier)) {}
  template <class T>
  StrikeoutClausesStorage(const T& s,
                          std::unique_ptr<unification::IUnificator> unifier)
      : unifier_(std::move(unifier)) {
    for (auto& c : s) {
      if (!Contains(c) && !unifier_->IsTautology(c)) {
        storage_.push_back(c);
      }
    }
  }

  std::optional<Clause> NextClause() override;

  bool Contains(const Clause& c) const override;

  bool IsPartOfExistentClause(const Clause& c) const;

  void FilterOutPartedClauses(const Clause& c);

  void AddClause(const Clause& c) override;

  bool empty() const override { return storage_.empty(); }

  std::vector<Clause> Infer(
      const Clause& c,
      const unification::IUnificator& unificator) const override;

 private:
  std::unique_ptr<unification::IUnificator> unifier_;
  StorageType storage_;
};
}  // namespace fol::types

