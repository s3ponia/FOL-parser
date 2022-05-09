#pragma once

#include <algorithm>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <memory>

namespace fol::prover {
class Prover {
 public:
  bool Provable() {
    while (!passive_clauses_->empty()) {
      auto o_current = passive_clauses_->NextClause();
      if (!o_current.has_value()) {
        break;
      }
      auto &current = *o_current;

      active_clauses_->AddClause(current);
      auto new_clauses = active_clauses_->Infer(current);

      if (std::any_of(new_clauses.begin(), new_clauses.end(),
                      [](auto &&clause) { return clause.empty(); })) {
        return true;
      }

      for (auto &&clause : new_clauses) {
        passive_clauses_->AddClause(clause);
      }
    }

    return false;
  }

 private:
  std::unique_ptr<unification::IUnificator> unificator_;
  std::unique_ptr<types::IClausesStorage> passive_clauses_;
  std::unique_ptr<types::IClausesStorage> active_clauses_;
};
}  // namespace fol::prover
