#include <libfol-basictypes/basic_clauses_storage.hpp>

namespace fol::types {
std::optional<Clause> BasicClausesStorage::NextClause() {
  if (storage_.empty()) {
    return std::nullopt;
  }
  auto ret = storage_.front();
  storage_.pop_front();
  return ret;
}

bool BasicClausesStorage::Contains(const Clause& c) const {
  return std::any_of(storage_.begin(), storage_.end(),
                     [&](auto&& cl) { return cl == c; });
}

void BasicClausesStorage::AddClause(const Clause& c) {
  if (!Contains(c)) {
    storage_.push_back(c);
  }
}

std::vector<Clause> BasicClausesStorage::Infer(
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
