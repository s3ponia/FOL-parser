#pragma once

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

  std::optional<types::Clause> Prove();

 private:
  std::unique_ptr<unification::IUnificator> unificator_;
  std::unique_ptr<types::IClausesStorage> passive_clauses_;
  std::unique_ptr<types::IClausesStorage> active_clauses_;
};
}  // namespace fol::prover
