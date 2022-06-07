#pragma once

#include <algorithm>
#include <libfol-basictypes/clauses_storage_interface.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <memory>
#include <optional>

namespace fol::prover {
class Prover {
 public:
  Prover(std::unique_ptr<unification::IUnificator> unificator,
         std::unique_ptr<types::IClausesStorage> passive_storage,
         std::unique_ptr<types::IClausesStorage> active_storage)
      : unificator_(std::move(unificator)),
        passive_clauses_(std::move(passive_storage)),
        active_clauses_(std::move(active_storage)) {}

  std::optional<types::Clause> Prove() {
    while (!passive_clauses_->empty()) {
      auto o_current = passive_clauses_->NextClause();
      if (!o_current.has_value()) {
        break;
      }
      std::cout << "Get clause: " << *o_current << std::endl;
      auto &current = *o_current;

      auto new_clauses = active_clauses_->Infer(current, *unificator_);
      active_clauses_->AddClause(current);
      auto empty_clause_it =
          std::find_if(new_clauses.begin(), new_clauses.end(),
                       [](auto &&clause) { return clause.empty(); });

      if (empty_clause_it != new_clauses.end()) {
        return *empty_clause_it;
      }

      for (auto &&clause : new_clauses) {
        if (!active_clauses_->Contains(clause)) {
          passive_clauses_->AddClause(clause);
        }
      }
    }

    return std::nullopt;
  }

 private:
  std::unique_ptr<unification::IUnificator> unificator_;
  std::unique_ptr<types::IClausesStorage> passive_clauses_;
  std::unique_ptr<types::IClausesStorage> active_clauses_;
};
}  // namespace fol::prover
