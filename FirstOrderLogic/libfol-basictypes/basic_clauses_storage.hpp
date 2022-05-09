#pragma once

#include <libfol-basictypes/clause.hpp>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <optional>
#include <vector>

namespace fol::types {
class BasicClausesStorage : public IClausesStorage {
 public:
  using StorageType = std::vector<Clause>;
  BasicClausesStorage(const StorageType& s) : storage_(s) {}

  std::optional<Clause> NextClause() override {
    if (storage_.empty()) {
      return std::nullopt;
    }
    auto ret = storage_.back();
    storage_.pop_back();
    return ret;
  }

  void AddClause(const Clause& c) override { storage_.push_back(c); }

  bool empty() const override { return storage_.empty(); }

  bool Simplify(const Clause&) override { return false; }

 private:
  StorageType storage_;
};
}  // namespace fol::types
