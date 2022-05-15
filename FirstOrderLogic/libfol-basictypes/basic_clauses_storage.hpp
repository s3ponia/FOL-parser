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
  BasicClausesStorage(const T& s) : storage_(s.begin(), s.end()) {}

  std::optional<Clause> NextClause() override {
    if (storage_.empty()) {
      return std::nullopt;
    }
    auto ret = storage_.front();
    storage_.pop_front();
    return ret;
  }

  void AddClause(const Clause& c) override {
    if (std::all_of(storage_.begin(), storage_.end(),
                    [&](auto&& cl) { return !(cl == c); })) {
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
  StorageType storage_;
};
}  // namespace fol::types
