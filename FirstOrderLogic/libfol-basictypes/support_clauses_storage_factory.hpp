#pragma once

#include <libfol-basictypes/clauses_storage_factory_interface.hpp>

namespace fol::types {
template <class T>
class SupportClausesStorageFactory : public T, public IClausesStorageFactory {
 public:
  std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis) override {
    return {std::make_unique<T>(std::move(hypothesis)),
            std::make_unique<T>(std::move(axioms))};
  }
};
}  // namespace fol::types
