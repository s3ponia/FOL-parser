#pragma once

#include <libfol-basictypes/clauses_storage_factory_interface.hpp>
#include <libfol-basictypes/strikeout_clauses_storage.hpp>
#include <libfol-unification/unification_factory_interface.hpp>

namespace fol::types {
template <class T>
class StrikeoutClausesStorageFactory : public IClausesStorageFactory {
 public:
  StrikeoutClausesStorageFactory(
      std::shared_ptr<unification::IUnificatorFactory> unifier)
      : unifier_factory_(std::move(unifier)) {}
  std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis) override {
    axioms.reserve(axioms.size() + hypothesis.size());
    axioms.insert(axioms.end(), hypothesis.begin(), hypothesis.end());
    return {std::make_unique<StrikeoutClausesStorage<T>>(
                std::move(axioms), unifier_factory_->create()),
            std::make_unique<StrikeoutClausesStorage<T>>(
                unifier_factory_->create())};
  }

 private:
  std::shared_ptr<unification::IUnificatorFactory> unifier_factory_;
};
}  // namespace fol::types
