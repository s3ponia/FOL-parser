#pragma once

#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>

#include "details/utils/utility.hpp"

namespace fol::transform {
inline parser::FolFormula Normalize(parser::FolFormula formula) {}

inline parser::FolFormula Copy(const parser::FolFormula &formula) {
  return parser::Parse(lexer::Tokenize(parser::ToString(formula)));
}

inline auto Match(auto matcher, auto formula) {
  matcher.match(std::move(formula));
  return std::move(matcher.formula.value());
}

inline parser::ImplicationFormula DropAllOutBrackets(
    parser::ImplicationFormula formula) {
  if (matcher::check::Brackets()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Brackets(matcher::RefImpl(impl)).match(std::move(formula));
    return std::move(impl.value());
  }

  return formula;
}

inline parser::ImplicationFormula DropAllBracketsInNot(
    parser::ImplicationFormula formula) {
  // ~(((...(F)...))) = ~(F)
  if (matcher::check::Not(matcher::check::Brackets())(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::Brackets(matcher::RefImpl(impl)))
        .match(std::move(formula));

    formula =
        matcher::UnaryToFol(~!DropAllOutBrackets(std::move(impl.value())));
  }
  return formula;
}

inline parser::ImplicationFormula ToConjunctionNormalForm(
    parser::ImplicationFormula formula) {
  formula = DropAllOutBrackets(std::move(formula));

  formula = DropAllBracketsInNot(std::move(formula));

  // Pred | ~Pred | [~]Pred or [~]Pred ::= <<end>>
  auto pred_checker = matcher::check::OrMatch(
      matcher::check::Pred(), matcher::check::Not(matcher::check::Pred()));
  if (pred_checker(formula) ||
      matcher::check::Disj(pred_checker, pred_checker)(formula)) {
    return formula;
  }

  // F -> G = (~ F) or (G)
  if (matcher::check::Impl(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> impl_f;
    std::optional<parser::ImplicationFormula> impl;

    matcher::Impl(matcher::RefImpl(impl_f), matcher::RefImpl(impl))
        .match(std::move(formula));

    parser::ImplicationFormula disj_t = ToConjunctionNormalForm(
        {~!std::move(impl_f.value()) || !std::move(impl.value())});

    return disj_t;
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

    auto conj = !ToConjunctionNormalForm(
                    matcher::UnaryToFol(~!std::move(impl_lhs.value()))) &&
                !ToConjunctionNormalForm(
                    matcher::UnaryToFol(~!std::move(impl_rhs.value())));

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

    auto disj = !ToConjunctionNormalForm(
                    matcher::UnaryToFol(~!std::move(impl_lhs.value()))) ||
                !ToConjunctionNormalForm(
                    matcher::UnaryToFol(~!std::move(impl_rhs.value())));

    return parser::MakeImpl(std::move(disj));
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
                          ToConjunctionNormalForm(
                              matcher::UnaryToFol(~!std::move(impl.value()))));
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
                          ToConjunctionNormalForm(
                              matcher::UnaryToFol(~!std::move(impl.value()))));
  }

  // F or (G and H) = ((F) or (G)) and ((F) or (H))
  if (matcher::check::Disj(
          matcher::check::Anything(),
          matcher::check::Conj(matcher::check::Anything(),
                               matcher::check::Anything()))(formula)) {
    std::optional<parser::ImplicationFormula> impl_f;
    std::optional<parser::ImplicationFormula> impl_g;
    std::optional<parser::ImplicationFormula> impl_h;

    matcher::Disj(matcher::RefImpl(impl_f),
                  matcher::Brackets(matcher::Conj(matcher::RefImpl(impl_g),
                                                  matcher::RefImpl(impl_h))))
        .match(std::move(formula));

    auto impl_f_c = Copy(impl_f.value());
    auto conj = !(!std::move(impl_f.value()) || !std::move(impl_g.value())) &&
                !(!std::move(impl_f_c) || !std::move(impl_h.value()));

    return ToConjunctionNormalForm({std::move(conj)});
  }

  // (G and H) or F = ((F) or (G)) and ((F) or (H))
  if (matcher::check::Disj(matcher::check::Conj(matcher::check::Anything(),
                                                matcher::check::Anything()),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> impl_f;
    std::optional<parser::ImplicationFormula> impl_g;
    std::optional<parser::ImplicationFormula> impl_h;

    matcher::Disj(matcher::Brackets(matcher::Conj(matcher::RefImpl(impl_g),
                                                  matcher::RefImpl(impl_h))),
                  matcher::RefImpl(impl_f))
        .match(std::move(formula));

    auto impl_f_c = Copy(impl_f.value());
    auto conj = !(!std::move(impl_f.value()) || !std::move(impl_g.value())) &&
                !(!std::move(impl_f_c) || !std::move(impl_h.value()));

    return ToConjunctionNormalForm({std::move(conj)});
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::RefImpl(impl)).match(std::move(formula));

    return ToConjunctionNormalForm(matcher::UnaryToFol(
        ~!ToConjunctionNormalForm(std::move(impl.value()))));
  }

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    std::optional<std::string> var_name;
    matcher::Forall(matcher::RefName(var_name), matcher::RefImpl(impl))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var_name.value();

    return parser::ForAll(std::move(var_v),
                          ToConjunctionNormalForm(std::move(impl.value())));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    std::optional<std::string> var_name;
    matcher::Exists(matcher::RefName(var_name), matcher::RefImpl(impl))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var_name.value();

    return parser::Exists(std::move(var_v),
                          ToConjunctionNormalForm(std::move(impl.value())));
  }

  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> lhs;
    std::optional<parser::ImplicationFormula> rhs;

    matcher::Disj(matcher::RefImpl(lhs), matcher::RefImpl(rhs))
        .match(std::move(formula));

    return ToConjunctionNormalForm(
        {!ToConjunctionNormalForm(std::move(lhs.value())) ||
         !ToConjunctionNormalForm(std::move(rhs.value()))});
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> lhs;
    std::optional<parser::ImplicationFormula> rhs;

    matcher::Conj(matcher::RefImpl(lhs), matcher::RefImpl(rhs))
        .match(std::move(formula));

    return {!ToConjunctionNormalForm(std::move(lhs.value())) &&
            !ToConjunctionNormalForm(std::move(rhs.value()))};
  }

  return formula;
}

inline parser::FolFormula DeleteUselessBrackets(parser::FolFormula formula) {
  formula = DropAllOutBrackets(std::move(formula));
  formula = DropAllBracketsInNot(std::move(formula));

  // ~(<pred>) = ~<pred>
  if (matcher::check::Not(matcher::check::Brackets(matcher::check::Pred()))(
          formula)) {
    std::optional<parser::PredicateFormula> pred;
    matcher::Not(matcher::Brackets(matcher::RefPred(pred)))
        .match(std::move(formula));

    return matcher::UnaryToFol(~std::move(pred.value()));
  }

  // (<pred>) = <pred>
  if (matcher::check::Brackets(matcher::check::Pred())(formula)) {
    std::optional<parser::PredicateFormula> pred;
    matcher::Brackets(matcher::RefPred(pred)).match(std::move(formula));

    return matcher::UnaryToFol(std::move(pred.value()));
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::RefImpl(impl)).match(std::move(formula));

    return matcher::UnaryToFol(
        ~!DeleteUselessBrackets(std::move(impl.value())));
  }

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    std::optional<std::string> var_name;
    matcher::Forall(matcher::RefName(var_name), matcher::RefImpl(impl))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var_name.value();

    return parser::ForAll(std::move(var_v),
                          DeleteUselessBrackets(std::move(impl.value())));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    std::optional<std::string> var_name;
    matcher::Exists(matcher::RefName(var_name), matcher::RefImpl(impl))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var_name.value();

    return parser::Exists(std::move(var_v),
                          DeleteUselessBrackets(std::move(impl.value())));
  }

  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> lhs;
    std::optional<parser::ImplicationFormula> rhs;

    matcher::Disj(matcher::RefImpl(lhs), matcher::RefImpl(rhs))
        .match(std::move(formula));

    return {!DeleteUselessBrackets(std::move(lhs.value())) ||
            !DeleteUselessBrackets(std::move(rhs.value()))};
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> lhs;
    std::optional<parser::ImplicationFormula> rhs;

    matcher::Conj(matcher::RefImpl(lhs), matcher::RefImpl(rhs))
        .match(std::move(formula));

    return {!DeleteUselessBrackets(std::move(lhs.value())) &&
            !DeleteUselessBrackets(std::move(rhs.value()))};
  }

  return DropAllOutBrackets(std::move(formula));
}

inline parser::FolFormula ToCNF(parser::FolFormula formula) {
  return DeleteUselessBrackets(ToConjunctionNormalForm(std::move(formula)));
}

}  // namespace fol::transform
