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

  std::optional<Clause> NextClause() override {
    if (storage_.empty()) {
      return std::nullopt;
    }
    auto ret = *storage_.begin();
    storage_.erase(storage_.begin());
    return ret;
  }

  bool Contains(const Clause& c) override { return storage_.contains(c); }

  void AddClause(const Clause& c) override {
    if (!Contains(c)) {
      storage_.insert(c);
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
  StorageType storage_;
};
}  // namespace fol::types
