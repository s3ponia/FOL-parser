#pragma once

#include <libfol-basictypes/term.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <list>

namespace fol::unification {
class MartelliMontanariUnificator : public IUnificator {
 public:
  using EqualitiesType = std::list<std::pair<types::Term, types::Term>>;
  enum class Result { OK, EMPTY, ERROR };

  std::optional<Substitution> Unificate(const types::Atom& lhs,
                                        const types::Atom& rhs) const override;
};
}  // namespace fol::unification
