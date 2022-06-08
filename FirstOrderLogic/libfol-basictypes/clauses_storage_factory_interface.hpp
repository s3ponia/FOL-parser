#pragma once

#include <libfol-basictypes/clauses_storage_interface.hpp>

namespace fol::types {
class IClausesStorageFactory {
 public:
  virtual std::pair<std::unique_ptr<IClausesStorage>,
                    std::unique_ptr<IClausesStorage>>
  create(std::vector<Clause> axioms, std::vector<Clause> hypothesis) = 0;
};
}  // namespace fol::types
