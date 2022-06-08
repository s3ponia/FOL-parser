#include <libfol-basictypes/basic_clauses_storage.hpp>
#include <libfol-basictypes/support_clauses_storage_factory.hpp>

namespace fol::types {
std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
SupportClausesStorageFactory::create(std::vector<Clause> axioms,
                                     std::vector<Clause> hypothesis) {
  return {std::make_unique<BasicClausesStorage>(std::move(hypothesis)),
          std::make_unique<BasicClausesStorage>(std::move(axioms))};
}
}  // namespace fol::types
