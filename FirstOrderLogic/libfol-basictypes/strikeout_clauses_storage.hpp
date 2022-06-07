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

  std::optional<Clause> NextClause() override {
    if (storage_.empty()) {
      return std::nullopt;
    }
    auto ret = storage_.front();
    storage_.pop_front();
    return ret;
  }

  bool Contains(const Clause& c) const override {
    return std::any_of(storage_.begin(), storage_.end(),
                       [&](auto&& cl) { return cl == c; });
  }

  bool IsPartOfExistentClause(const Clause& c) const {
    return std::any_of(storage_.begin(), storage_.end(),
                       [&](auto&& cl) { return unifier_->IsPartOf(cl, c); });
  }

  void FilterOutPartedClauses(const Clause& c) {
    for (auto it = storage_.begin(); it != storage_.end();) {
      if (unifier_->IsPartOf(c, *it)) {
        it = storage_.erase(it);
      } else {
        ++it;
      }
    }
  }

  void AddClause(const Clause& c) override {
    if (!Contains(c) && !IsPartOfExistentClause(c) &&
        !unifier_->IsTautology(c)) {
      storage_.push_back(c);
    }
  }

  bool empty() const override { return storage_.empty(); }

  bool Simplify(const Clause&) override { return false; }

  std::vector<Clause> Infer(
      const Clause& c,
      const unification::IUnificator& unificator) const override {
    std::vector<Clause> res;

    for (auto& c_s : storage_) {
      auto o_c = unificator.Resolution(c, c_s);
      if (o_c) {
        res.push_back(*o_c);
      }
    }

    return res;
  }

 private:
  std::unique_ptr<unification::IUnificator> unifier_;
  StorageType storage_;
};
}  // namespace fol::types

