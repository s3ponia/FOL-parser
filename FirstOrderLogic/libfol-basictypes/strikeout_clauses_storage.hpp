#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "libfol-unification/unification_interface.hpp"

namespace fol::types {
template <class ClauseStorage>
class StrikeoutClausesStorage : public IClausesStorage {
 public:
  using StorageType = ClauseStorage;
  StrikeoutClausesStorage(std::unique_ptr<unification::IUnificator> unifier)
      : unifier_(std::move(unifier)) {}
  template <class T>
  StrikeoutClausesStorage(const T& s,
                          std::unique_ptr<unification::IUnificator> unifier)
      : unifier_(std::move(unifier)) {
    for (auto& c : s) {
      if (!Contains(c) && !unifier_->IsTautology(c)) {
        storage_.AddClause(c);
      }
    }
  }

  std::optional<Clause> NextClause() override { return storage_.NextClause(); }

  bool Contains(const Clause& c) const override { return storage_.Contains(c); }

  bool IsPartOfExistentClause(const Clause& c) const {
    return std::any_of(storage_.begin(), storage_.end(),
                       [&](auto&& cl) { return unifier_->IsPartOf(cl, c); });
  }

  void AddClause(const Clause& c) override {
    if (!Contains(c) && !IsPartOfExistentClause(c) &&
        !unifier_->IsTautology(c)) {
      storage_.AddClause(c);
    }
  }

  bool empty() const override { return storage_.empty(); }

  std::vector<Clause> Infer(
      const Clause& c,
      const unification::IUnificator& unificator) const override {
    return storage_.Infer(c, unificator);
  }

  auto begin() const { return storage_.begin(); }

  auto end() const { return storage_.end(); }

 private:
  std::unique_ptr<unification::IUnificator> unifier_;
  StorageType storage_;
};
}  // namespace fol::types

