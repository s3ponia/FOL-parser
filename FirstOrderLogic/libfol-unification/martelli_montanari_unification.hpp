#pragma once

#include <libfol-basictypes/term.hpp>
#include <libfol-unification/unification_interface.hpp>
#include <list>
#include <utility>

namespace fol::unification {
class MartelliMontanariUnificator : public IUnificator {
  using EqualitiesType = std::list<std::pair<types::Term, types::Term>>;
  enum class Result { OK, EMPTY, ERROR };

  static Result ApplyRules(EqualitiesType& equalities) {
    using types::operator==;
    for (auto eq_it = equalities.begin(); eq_it != equalities.end(); ++eq_it) {
      auto& eq = *eq_it;
      // RULE 1
      if (eq.first.IsFunction() && eq.second.IsFunction()) {
        if (FunctionName(eq.first.Function()) !=
            FunctionName(eq.second.Function())) {
          return Result::ERROR;
        }

        auto lhs_termlist =
            parser::FunctionTerms(std::move(eq.first.Function()));
        auto rhs_termlist =
            parser::FunctionTerms(std::move(eq.second.Function()));

        if (lhs_termlist.size() != rhs_termlist.size()) {
          return Result::ERROR;
        }

        for (std::size_t i = 0; i < lhs_termlist.size(); ++i) {
          equalities.emplace_back(std::move(lhs_termlist[i]),
                                  std::move(rhs_termlist[i]));
        }

        equalities.erase(eq_it);

        return Result::OK;
      }
      // RULE 1 END

      // RULE 2
      if (!eq.first.IsVar() && eq.second.IsVar()) {
        equalities.emplace_back(std::move(eq.second), std::move(eq.first));
        equalities.erase(eq_it);
        return Result::OK;
      }
      // RULE 2 END

      // RULE 3
      if (eq.first == eq.second) {
        equalities.erase(eq_it);
        return Result::OK;
      }
      // RULE 3 END

      // RULE 4
      if (eq.first.IsVar() && !types::Contains(eq.second, eq.first)) {
        auto substitution =
            Substitution{{{eq.first.Var(), std::move(eq.second)}}};

        auto flag_contains = false;

        for (auto i = equalities.begin(); i != equalities.end(); ++i) {
          if (i != eq_it) {
            flag_contains |= types::Contains(i->first, eq.first) ||
                             types::Contains(i->second, eq.first);
            substitution.Substitute(i->first);
            substitution.Substitute(i->second);
          }
        }

        if (flag_contains) {
          return Result::OK;
        } else {
          continue;
        }
      }
      // RULE 4 END
    }

    return Result::EMPTY;
  }

 public:
  std::optional<Substitution> Unificate(const types::Atom& lhs,
                                        const types::Atom& rhs) const override {
    if (lhs.predicate_name() != rhs.predicate_name() ||
        lhs.terms_size() != rhs.terms_size()) {
      return std::nullopt;
    }

    EqualitiesType equalities;

    for (std::size_t i = 0; i < lhs.terms_size(); ++i) {
      equalities.emplace_back(types::Clone(lhs[i]), types::Clone(rhs[i]));
    }

    Result res;

    while ((res = ApplyRules(equalities)) == Result::OK) {
    }

    if (res == Result::ERROR) {
      return std::nullopt;
    }

    std::vector<Substitution::SubstitutePair> pairs;
    pairs.reserve(equalities.size());

    for (auto& eq : equalities) {
      pairs.emplace_back(eq.first.Var(), std::move(eq.second));
    }

    return pairs;
  }
};
}  // namespace fol::unification
