#include <libfol-unification/unification_interface.hpp>

namespace fol::unification {
void IUnificator::Simplify(types::Clause& clause) const {
  auto& atoms_ = clause.atoms();
  for (std::vector<types::Atom>::size_type i = 0; i < clause.atoms().size();
       ++i) {
    for (std::vector<types::Atom>::size_type j = i + 1;
         j < clause.atoms().size(); ++j) {
      if (auto substitution = this->Unificate(atoms_[i], atoms_[j])) {
        substitution.value().Substitute(clause);
      }
    }
  }

  std::vector<types::Atom> res_;
  res_.reserve(atoms_.size());

  for (std::vector<types::Atom>::size_type i = 0; i < atoms_.size(); ++i) {
    bool flag_uniq = true;
    for (std::vector<types::Atom>::size_type j = i + 1; j < atoms_.size();
         ++j) {
      if (atoms_[i] == atoms_[j]) {
        flag_uniq = false;
        break;
      }
    }

    if (flag_uniq) {
      res_.push_back(atoms_[i]);
    }
  }

  atoms_ = std::move(res_);
  std::sort(atoms_.begin(), atoms_.end());
}

bool IUnificator::IsPartOf(const types::Clause& lhs,
                           const types::Clause& rhs) const {
  for (auto& lhs_atom : lhs.atoms()) {
    bool contains = false;
    for (auto& rhs_atom : rhs.atoms()) {
      if (rhs_atom.negative() != lhs_atom.negative()) {
        continue;
      }
      if (auto sub = Unificate(lhs_atom, rhs_atom)) {
        contains = true;
        break;
      }
    }
    if (!contains) {
      return false;
    }
  }

  return true;
}

std::optional<types::Clause> IUnificator::Resolution(types::Clause lhs,
                                                     types::Clause rhs) const {
  std::cout << "Try resolve: " << lhs << " <<<>>> " << rhs << std::endl;
  for (std::size_t i = 0; i < lhs.atoms().size(); ++i) {
    for (std::size_t j = 0; j < rhs.atoms().size(); ++j) {
      if ((lhs.atoms()[i].negative() + rhs.atoms()[j].negative()) % 2 == 1) {
        auto sub = Unificate(lhs.atoms()[i], rhs.atoms()[j]);

        if (!sub) {
          continue;
        }

        auto cpy_lhs = lhs;
        auto cpy_rhs = rhs;

        cpy_lhs.EraseAtom(i);
        cpy_rhs.EraseAtom(j);

        cpy_lhs += cpy_rhs;

        sub->Substitute(cpy_lhs);

        Simplify(cpy_lhs);

        std::cout << "Resolution: " << lhs << " RESOLVE " << rhs << " >>> "
                  << cpy_lhs << std::endl;

        cpy_lhs.ClearAncestors();
        cpy_lhs.GenerateId();
        cpy_lhs.AddAncestor(lhs);
        cpy_lhs.AddAncestor(rhs);

        return cpy_lhs;
      }
    }
  }
  return std::nullopt;
}

bool IUnificator::IsTautology(const types::Clause& c) const {
  for (auto& a_1 : c.atoms()) {
    for (auto& a_2 : c.atoms()) {
      if (a_1.negative() != a_2.negative() && Unificate(a_1, a_2)) {
        return true;
      }
    }
  }
  return false;
}
}  // namespace fol::unification
