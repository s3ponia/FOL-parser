#include <libfol-basictypes/strikeout_clauses_storage.hpp>

namespace fol::types {
std::optional<Clause> StrikeoutClausesStorage::NextClause() {
  if (storage_.empty()) {
    return std::nullopt;
  }
  auto ret = storage_.front();
  storage_.pop_front();
  return ret;
}

bool StrikeoutClausesStorage::Contains(const Clause& c) const {
  return std::any_of(storage_.begin(), storage_.end(),
                     [&](auto&& cl) { return cl == c; });
}

bool StrikeoutClausesStorage::IsPartOfExistentClause(const Clause& c) const {
  return std::any_of(storage_.begin(), storage_.end(),
                     [&](auto&& cl) { return unifier_->IsPartOf(cl, c); });
}

void StrikeoutClausesStorage::FilterOutPartedClauses(const Clause& c) {
  for (auto it = storage_.begin(); it != storage_.end();) {
    if (unifier_->IsPartOf(c, *it)) {
      it = storage_.erase(it);
    } else {
      ++it;
    }
  }
}

void StrikeoutClausesStorage::AddClause(const Clause& c) {
  if (!Contains(c) && !IsPartOfExistentClause(c) && !unifier_->IsTautology(c)) {
    storage_.push_back(c);
  }
}

std::vector<Clause> StrikeoutClausesStorage::Infer(
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
