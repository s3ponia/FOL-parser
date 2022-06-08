#pragma once

#include <libfol-basictypes/function.hpp>
#include <libfol-basictypes/term.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <optional>

#include "libfol-unification/substitution.hpp"

namespace fol::unification {
class RobinsonUnificator : public IUnificator {
 public:
  std::optional<Substitution> Unificate(const types::Atom& lhs,
                                        const types::Atom& rhs) const override;
};
}  // namespace fol::unification
