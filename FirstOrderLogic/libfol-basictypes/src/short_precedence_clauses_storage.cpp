#include <libfol-basictypes/short_precedence_clauses_storage.hpp>

namespace fol::types {
std::optional<Clause> ShortPrecedenceClausesStorage::NextClause() {
  if (storage_.empty()) {
    return std::nullopt;
  }
  auto ret = *storage_.begin();
  storage_.erase(storage_.begin());
  return ret;
}

void ShortPrecedenceClausesStorage::AddClause(const Clause& c) {
  if (!Contains(c)) {
    storage_.insert(c);
  }
}

bool ShortPrecedenceClausesStorage::empty() const { return storage_.empty(); }

std::vector<Clause> ShortPrecedenceClausesStorage::Infer(
    const Clause& c, const unification::IUnificator& unificator) const {
  std::vector<Clause> res;

  for (auto& c_s : storage_) {
    auto o_c = unificator.Resolution(c, c_s);
    if (o_c) {
      res.push_back(*o_c);
    }
  }

  return res;
}
}  // namespace fol::types
