#include <algorithm>
#include <libfol-prover/prover.hpp>

namespace fol::prover {
std::optional<types::Clause> Prover::Prove() {
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
}  // namespace fol::prover
