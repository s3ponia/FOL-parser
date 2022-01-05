#pragma once

#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/parser/types.hpp>

#include "details/utils/utility.hpp"

namespace fol::transform {
inline parser::FolFormula Normalize(parser::FolFormula formula) {}

inline parser::ImplicationFormula ToConjunctionNormalForm(
    parser::ImplicationFormula);

inline parser::UnaryFormula ToConjunctionNormalForm(
    parser::UnaryFormula formula) {
  // ~(~F) = F
  if (matcher::check::CheckNot(matcher::check::CheckNot())(formula)) {
    std::optional<parser::UnaryFormula> fol_formula;
    matcher::Not(matcher::Not(matcher::RefUnary(fol_formula)))
        .match(std::move(formula));
    return std::move(fol_formula.value());
  }

  // ~(F or G) = ~F and ~G
}

inline parser::ConjunctionFormula ToConjunctionNormalForm(
    parser::ConjunctionFormula formula) {
  std::optional<parser::UnaryFormula> unary;
  std::optional<parser::ConjunctionFormula> conj;
  if (matcher::Conj(matcher::RefUnary(unary), matcher::RefConj(conj))
          .match(matcher::DisjToFol(std::move(formula)))) {
    return parser::MakeConj(ToConjunctionNormalForm(std::move(unary.value())),
                            std::move(conj.value()));
  }

  return parser::MakeConj(ToConjunctionNormalForm(std::move(unary.value())));
}

inline parser::DisjunctionFormula ToConjunctionNormalForm(
    parser::DisjunctionFormula formula) {
  std::optional<parser::ConjunctionFormula> conj;
  std::optional<parser::DisjunctionFormula> disj;

  // If not match then it matches <conj> <EPS> and saves <conj> to first param
  if (matcher::Disj(matcher::RefConj(conj), matcher::RefDisj(disj))
          .match(matcher::DisjToFol(std::move(formula)))) {
    return parser::MakeDisj(ToConjunctionNormalForm(std::move(conj.value())),
                            ToConjunctionNormalForm(std::move(disj.value())));
  }

  return parser::MakeDisj(ToConjunctionNormalForm(std::move(conj.value())));
}

inline parser::ImplicationFormula ToConjunctionNormalForm(
    parser::ImplicationFormula formula) {
  std::optional<parser::DisjunctionFormula> disj;
  std::optional<parser::ImplicationFormula> impl;
  // If not match then it saves to disj
  // F -> G = not F or G
  if (matcher::Impl(matcher::RefDisj(disj), matcher::RefImpl(impl))
          .match(std::move(formula))) {
    return parser::MakeImpl(parser::MakeDisj(
        parser::MakeConj(
            {parser::MakeNot({parser::MakeBrackets(parser::MakeImpl(
                ToConjunctionNormalForm(std::move(disj.value()))))})}),
        std::get<parser::DisjunctionFormula>(
            ToConjunctionNormalForm(std::move(impl.value())).data)));
  }
  return parser::MakeImpl(ToConjunctionNormalForm(std::move(disj.value())));
}

inline parser::FolFormula ToCNF(parser::FolFormula formula) {
  return ToConjunctionNormalForm(std::move(formula));
}

}  // namespace fol::transform
