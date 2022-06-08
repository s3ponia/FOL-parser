#include <libfol-basictypes/short_precedence_clauses_storage.hpp>
#include <libfol-basictypes/short_precedence_clauses_storage_factory.hpp>

namespace fol::types {
std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
ShortPrecedenceClausesStorageFactory::create(std::vector<Clause> axioms,
                                             std::vector<Clause> hypothesis) {
  axioms.reserve(axioms.size() + hypothesis.size());
  axioms.insert(axioms.end(), hypothesis.begin(), hypothesis.end());
  return {std::make_unique<ShortPrecedenceClausesStorage>(std::move(axioms)),
          std::make_unique<ShortPrecedenceClausesStorage>()};
}
}  // namespace fol::types
