#pragma once

#include <libfol-basictypes/clauses_storage_factory_interface.hpp>

namespace fol::types {
class BasicClausesStorageFactory : public IClausesStorageFactory {
 public:
  std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis) override;
};
}  // namespace fol::types
