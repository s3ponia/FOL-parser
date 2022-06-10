#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <optional>
#include <set>
#include <vector>

#include "libfol-unification/unification_interface.hpp"

namespace fol::types {
class ShortPrecedenceClausesStorage : public IClausesStorage {
  struct ClauseComparator {
    bool operator()(const Clause& lhs, const Clause& rhs) const noexcept {
      return lhs.atoms().size() < rhs.atoms().size() ||
             (lhs.atoms().size() == rhs.atoms().size() && lhs < rhs);
    }
  };

 public:
  using StorageType = std::set<Clause, ClauseComparator>;
  ShortPrecedenceClausesStorage() = default;
  template <class T>
  ShortPrecedenceClausesStorage(const T& s) : storage_(s.begin(), s.end()) {}

  std::optional<Clause> NextClause() override;

  bool Contains(const Clause& c) const override { return storage_.contains(c); }

  void AddClause(const Clause& c) override;

  auto begin() const { return storage_.begin(); }

  auto end() const { return storage_.end(); }

  bool empty() const override;

  std::vector<Clause> Infer(
      const Clause& c,
      const unification::IUnificator& unificator) const override;

 private:
  StorageType storage_;
};
}  // namespace fol::types
