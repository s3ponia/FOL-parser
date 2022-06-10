#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <list>
#include <optional>
#include <vector>

#include "libfol-unification/unification_interface.hpp"

namespace fol::types {
class BasicClausesStorage : public IClausesStorage {
 public:
  using StorageType = std::list<Clause>;
  BasicClausesStorage() = default;
  template <class T>
  BasicClausesStorage(const T& s) {
    for (auto& c : s) {
      AddClause(c);
    }
  }

  std::optional<Clause> NextClause() override;

  bool Contains(const Clause& c) const override;

  void AddClause(const Clause& c) override;

  auto begin() const { return storage_.begin(); }

  auto end() const { return storage_.end(); }

  bool empty() const override { return storage_.empty(); }

  std::vector<Clause> Infer(
      const Clause& c,
      const unification::IUnificator& unificator) const override;

 private:
  StorageType storage_;
};
}  // namespace fol::types
