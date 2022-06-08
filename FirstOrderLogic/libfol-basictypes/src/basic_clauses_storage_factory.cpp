#include <libfol-basictypes/basic_clauses_storage.hpp>
#include <libfol-basictypes/basic_clauses_storage_factory.hpp>

namespace fol::types {

std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
BasicClausesStorageFactory::create(std::vector<Clause> axioms,
                                   std::vector<Clause> hypothesis) {
  axioms.reserve(hypothesis.size() + axioms.size());
  axioms.insert(axioms.end(), hypothesis.begin(), hypothesis.end());
  return {std::make_unique<BasicClausesStorage>(std::move(axioms)),
          std::make_unique<BasicClausesStorage>()};
}
}  // namespace fol::types
