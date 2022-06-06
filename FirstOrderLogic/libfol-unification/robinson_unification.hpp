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
                                        const types::Atom& rhs) const override {
    using fol::types::operator==;

    if (lhs.predicate_name() != rhs.predicate_name() ||
        lhs.terms().size() != rhs.terms().size()) {
      return std::nullopt;
    }

    Substitution res;

    for (std::size_t i = 0; i < lhs.terms().size(); ++i) {
      auto t1_cp = types::Clone(lhs.terms()[i]);
      auto t2_cp = types::Clone(rhs.terms()[i]);
      res.Substitute(t1_cp);
      res.Substitute(t2_cp);

      if (t1_cp == t2_cp) {
        continue;
      }

      if (t1_cp.IsVar() && t2_cp.IsVar()) {
        res += Substitution({{t1_cp.Var(), t2_cp}});
      }

      if (t1_cp.IsVar() && !t2_cp.IsVar()) {
        // t2_cp contains t1_cp
        if (types::Contains(t2_cp, t1_cp)) {
          return std::nullopt;
        }

        res += Substitution({{t1_cp.Var(), t2_cp}});
      }

      if (t2_cp.IsVar() && !t1_cp.IsVar()) {
        // t1_cp contains t2_cp
        if (types::Contains(t1_cp, t2_cp)) {
          return std::nullopt;
        }

        res += Substitution({{t2_cp.Var(), t1_cp}});
      }

      if (t1_cp.IsFunction() && t2_cp.IsFunction()) {
        auto sub =
            Unificate(std::move(t1_cp.Function()), std::move(t2_cp.Function()));

        if (sub.has_value()) {
          res += *sub;
        } else {
          return std::nullopt;
        }
      }

      if (t1_cp.IsConstant() && t2_cp.IsConstant() &&
          t1_cp.Const() != t2_cp.Const()) {
        return std::nullopt;
      }
    }

    return res;
  }

 private:
  std::optional<Substitution> Unificate(const types::Function& lhs,
                                        const types::Function& rhs) const {
    using fol::types::operator==;

    if (lhs.function_name() != rhs.function_name()) {
      return std::nullopt;
    }
    Substitution res;

    for (const types::Term& t1 : lhs.terms()) {
      for (const types::Term& t2 : rhs.terms()) {
        auto t1_cp = types::Clone(t1);
        auto t2_cp = types::Clone(t2);
        res.Substitute(t1_cp);
        res.Substitute(t2_cp);

        if (t1_cp == t2_cp) {
          continue;
        }

        if (t1_cp.IsVar() && !t2_cp.IsVar()) {
          // t2_cp contains t1_cp
          if (types::Contains(t2_cp, t1_cp)) {
            return std::nullopt;
          }

          res += Substitution({{t1_cp.Var(), t2_cp}});
        }

        if (t2_cp.IsVar() && !t1_cp.IsVar()) {
          // t1_cp contains t2_cp
          if (types::Contains(t1_cp, t2_cp)) {
            return std::nullopt;
          }

          res += Substitution({{t2_cp.Var(), t1_cp}});
        }

        if (t1_cp.IsFunction() && t2_cp.IsFunction()) {
          auto sub = Unificate(std::move(t1_cp.Function()),
                               std::move(t2_cp.Function()));

          if (sub.has_value()) {
            res += *sub;
          } else {
            return std::nullopt;
          }
        }

        if (t1_cp.IsConstant() && t2_cp.IsConstant() &&
            t1_cp.Const() != t2_cp.Const()) {
          return std::nullopt;
        }
      }
    }

    return res;
  }
};
}  // namespace fol::unification
