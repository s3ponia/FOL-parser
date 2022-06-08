#pragma once

#include <libfol-unification/unification_interface.hpp>
#include <stdexcept>

namespace fol::unification {
class HEREUnificator : public IUnificator {
 public:
  struct nil {};
  struct done {};
  struct loop {};
  enum VarRes { NIL = 0, TERM = 1, DONE = 2 };
  enum SubRes { LOOP = 2 };
  struct ExitClash : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
  struct ExitLoop : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  std::optional<Substitution> Unificate(const types::Atom& lhs,
                                        const types::Atom& rhs) const override;
};
}  // namespace fol::unification
