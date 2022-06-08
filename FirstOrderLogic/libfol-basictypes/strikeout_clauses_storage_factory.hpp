#pragma once

#include <libfol-basictypes/clauses_storage_factory_interface.hpp>
#include <libfol-unification/unification_factory_interface.hpp>

namespace fol::types {
class StrikeoutClausesStorageFactory : public IClausesStorageFactory {
 public:
  StrikeoutClausesStorageFactory(
      std::shared_ptr<unification::IUnificatorFactory> unifier)
      : unifier_factory_(std::move(unifier)) {}
  std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis) override;

 private:
  std::shared_ptr<unification::IUnificatorFactory> unifier_factory_;
};
}  // namespace fol::types
