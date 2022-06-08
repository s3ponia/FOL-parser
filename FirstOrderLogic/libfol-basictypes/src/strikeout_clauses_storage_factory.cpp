#include <libfol-basictypes/strikeout_clauses_storage.hpp>
#include <libfol-basictypes/strikeout_clauses_storage_factory.hpp>

namespace fol::types {
std::pair<std::unique_ptr<IClausesStorage>, std::unique_ptr<IClausesStorage>>
StrikeoutClausesStorageFactory::create(std::vector<Clause> axioms,
                                       std::vector<Clause> hypothesis) {
  axioms.reserve(axioms.size() + hypothesis.size());
  axioms.insert(axioms.end(), hypothesis.begin(), hypothesis.end());
  return {
      std::make_unique<StrikeoutClausesStorage>(std::move(axioms),
                                                unifier_factory_->create()),
      std::make_unique<StrikeoutClausesStorage>(unifier_factory_->create())};
}
}  // namespace fol::types
