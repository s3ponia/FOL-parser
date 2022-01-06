#pragma once

#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/parser/types.hpp>

#include "details/utils/utility.hpp"

namespace fol::transform {
inline parser::FolFormula Normalize(parser::FolFormula formula) {}

inline auto Match(auto matcher, auto formula) {
  matcher.match(std::move(formula));
  return std::move(matcher.formula.value());
}

inline parser::ImplicationFormula ToConjunctionNormalForm(
    parser::ImplicationFormula formula) {
  if (matcher::check::Pred()(formula)) {
    return formula;
  }

  // F -> G = ~ F or (G)
  if (matcher::check::Impl(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::UnaryFormula> unary;
    std::optional<parser::ImplicationFormula> impl;

    matcher::Impl(matcher::RefUnary(unary), matcher::RefImpl(impl))
        .match(std::move(formula));

    parser::DisjunctionFormula disj_t =
        ~Match(matcher::Unary(),
               ToConjunctionNormalForm(std::move(unary.value()))) ||
        !ToConjunctionNormalForm(std::move(impl.value()));

    return {std::move(disj_t)};
  }

  // ~(~F) = F
  if (matcher::check::Not(matcher::check::Not())(formula)) {
    std::optional<parser::ImplicationFormula> unary;
    matcher::Not(matcher::Not(matcher::RefImpl(unary)))
        .match(std::move(formula));

    return ToConjunctionNormalForm(std::move(unary.value()));
  }

  // ~(F or G) = ~F and ~G
  if (matcher::check::Not(matcher::check::Disj(
          matcher::check::Anything(), matcher::check::Anything()))(formula)) {
    std::optional<parser::ImplicationFormula> impl_lhs;
    std::optional<parser::ImplicationFormula> impl_rhs;

    matcher::Not(matcher::Brackets(matcher::Disj(matcher::RefImpl(impl_lhs),
                                                 matcher::RefImpl(impl_rhs))))
        .match(std::move(formula));

    auto conj = ~!ToConjunctionNormalForm(std::move(impl_lhs.value())) &&
                ~!ToConjunctionNormalForm(std::move(impl_rhs.value()));

    return parser::MakeImpl(parser::MakeDisj(std::move(conj)));
  }

  // ~(F and G) = ~F or ~G
  if (matcher::check::Not(matcher::check::Conj(
          matcher::check::Anything(), matcher::check::Anything()))(formula)) {
    std::optional<parser::ImplicationFormula> impl_lhs;
    std::optional<parser::ImplicationFormula> impl_rhs;

    matcher::Not(matcher::Brackets(matcher::Conj(matcher::RefImpl(impl_lhs),
                                                 matcher::RefImpl(impl_rhs))))
        .match(std::move(formula));

    auto conj = ~!ToConjunctionNormalForm(std::move(impl_lhs.value())) ||
                ~!ToConjunctionNormalForm(std::move(impl_rhs.value()));

    return parser::MakeImpl(std::move(conj));
  }

  // ~(@ vx . F) = ? vx . ~F
  if (matcher::check::Not(matcher::check::Forall())(formula)) {
    std::optional<std::string> var;
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::Forall(matcher::RefName(var), matcher::RefImpl(impl)))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var.value();

    return parser::Exists(std::move(var_v),
                          matcher::UnaryToFol(~!ToConjunctionNormalForm(
                              std::move(impl.value()))));
  }

  // ~(? vx . F) = @ vx . ~F
  if (matcher::check::Not(matcher::check::Exists())(formula)) {
    std::optional<std::string> var;
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::Exists(matcher::RefName(var), matcher::RefImpl(impl)))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var.value();

    return parser::ForAll(std::move(var_v),
                          matcher::UnaryToFol(~!ToConjunctionNormalForm(
                              std::move(impl.value()))));
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::RefImpl(impl)).match(std::move(formula));
    return matcher::UnaryToFol(
        ~!ToConjunctionNormalForm(std::move(impl.value())));
  }
}

inline parser::FolFormula ToCNF(parser::FolFormula formula) {
  return ToConjunctionNormalForm(std::move(formula));
}

}  // namespace fol::transform
