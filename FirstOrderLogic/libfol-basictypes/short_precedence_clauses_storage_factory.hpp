#pragma once

#include <libfol-basictypes/clauses_storage_factory_interface.hpp>

namespace fol::types {
class ShortPrecedenceClausesStorageFactory : public IClausesStorageFactory {
 public:
  std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis);
};
}  // namespace fol::types
