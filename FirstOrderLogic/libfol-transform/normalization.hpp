#pragma once

#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-transform/replace.hpp>

#include "details/utils/utility.hpp"

namespace fol::transform {
inline parser::FolFormula Normalize(parser::FolFormula formula);

inline bool IsNoQuantifiers(const parser::FolFormula &formula) {
  auto str = ToString(formula);

  return (str.find("@") == std::string::npos) &&
         (str.find("?") == std::string::npos);
}

inline bool IsAllDisj(const parser::DisjunctionFormula &formula) {
  auto str = ToString(formula);

  for (std::string::size_type i = 0; i < str.size(); ++i) {
    if (str[i] == 'a' || str[i] == 'n' || str[i] == 'd' || str[i] == '-' ||
        str[i] == '>') {
      return false;
    }

    if (str[i] == '~' && str[i] == '(') {
      return false;
    }
  }

  return true;
}

inline parser::ImplicationFormula DeleteUselessBrackets(
    parser::ImplicationFormula formula);
inline parser::DisjunctionFormula DeleteUselessBrackets(
    parser::DisjunctionFormula formula);

inline parser::FolFormula DeleteUselessBrackets(parser::FolFormula formula);

inline parser::FolFormula Copy(const parser::FolFormula &formula) {
  return parser::Parse(lexer::Tokenize(parser::ToString(formula)));
}

inline auto Match(auto matcher, parser::FolFormula formula) {
  matcher.match(std::move(formula));
  return std::move(matcher.formula.value());
}

inline parser::DisjunctionFormula DropAllOutBrackets(
    parser::DisjunctionFormula formula) {
  if (matcher::check::Brackets(matcher::check::Disj())(formula)) {
    std::optional<parser::DisjunctionFormula> impl;
    matcher::Brackets(matcher::RefDisj(impl)).match({std::move(formula)});
    return std::move(impl.value());
  }

  return formula;
}

inline parser::ConjunctionFormula DropAllOutBrackets(
    parser::ConjunctionFormula formula) {
  if (matcher::check::Brackets(matcher::check::Conj())(formula)) {
    std::optional<parser::ConjunctionFormula> impl;
    matcher::Brackets(matcher::RefConj(impl)).match({std::move(formula)});
    return std::move(impl.value());
  }

  return formula;
}

inline parser::ImplicationFormula DropAllOutBrackets(
    parser::ImplicationFormula formula) {
  if (matcher::check::Brackets(matcher::check::Impl())(formula)) {
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

// (Q vx . F[vx]) or G = (Q uc . F[uc] or G)
inline parser::ImplicationFormula QuantifiersToCNF1(
    parser::ImplicationFormula formula) {
  // G or (@ vx . F[vx]) = @ uc . F[uc] or G
  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::FolFormula> g;
    matcher::Disj(matcher::RefImpl(g), matcher::RefForall(forall_f))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first,
        {!std::move(forall_f.value().data.second) || !std::move(g.value())})))};
  }

  // G or (? vx . F[vx]) = ? uc . F[uc] or G
  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ExistsFormula> exists_f;
    std::optional<parser::FolFormula> g;
    matcher::Disj(matcher::RefImpl(g), matcher::RefExists(exists_f))
        .match(std::move(formula));

    exists_f = RenameVar(std::move(exists_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        exists_f.value().data.first,
        {!std::move(exists_f.value().data.second) || !std::move(g.value())})))};
  }

  // (@ vx . F[vx]) or G = @ uc . F[uc] or G
  if (matcher::check::Disj(matcher::check::Forall(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::FolFormula> g;
    matcher::Disj(matcher::RefForall(forall_f), matcher::RefImpl(g))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first,
        {!std::move(forall_f.value().data.second) || !std::move(g.value())})))};
  }

  // (? vx . F[vx]) or G = ? uc . F[uc] or G
  if (matcher::check::Disj(matcher::check::Exists(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ExistsFormula> exists_f;
    std::optional<parser::FolFormula> g;
    matcher::Disj(matcher::RefExists(exists_f), matcher::RefImpl(g))
        .match(std::move(formula));

    exists_f = RenameVar(std::move(exists_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        exists_f.value().data.first,
        {!std::move(exists_f.value().data.second) || !std::move(g.value())})))};
  }

  return formula;
}

// (Q vx . F[vx]) and G = (Q uc . F[uc] and G)
inline parser::ImplicationFormula QuantifiersToCNF2(
    parser::ImplicationFormula formula) {
  // G and (@ vx . F[vx]) = @ uc . F[uc] and G
  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::FolFormula> g;
    matcher::Conj(matcher::RefImpl(g), matcher::RefForall(forall_f))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first,
        {!std::move(forall_f.value().data.second) && !std::move(g.value())})))};
  }

  // G and (? vx . F[vx]) = ? uc . F[uc] and G
  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ExistsFormula> exists_f;
    std::optional<parser::FolFormula> g;
    matcher::Conj(matcher::RefImpl(g), matcher::RefExists(exists_f))
        .match(std::move(formula));

    exists_f = RenameVar(std::move(exists_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        exists_f.value().data.first,
        {!std::move(exists_f.value().data.second) && !std::move(g.value())})))};
  }

  // (@ vx . F[vx]) and G = @ uc . F[uc] and G
  if (matcher::check::Conj(matcher::check::Forall(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::FolFormula> g;
    matcher::Conj(matcher::RefForall(forall_f), matcher::RefImpl(g))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first,
        {!std::move(forall_f.value().data.second) && !std::move(g.value())})))};
  }

  // (? vx . F[vx]) and G = ? uc . F[uc] and G
  if (matcher::check::Conj(matcher::check::Exists(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ExistsFormula> exists_f;
    std::optional<parser::FolFormula> g;
    matcher::Conj(matcher::RefExists(exists_f), matcher::RefImpl(g))
        .match(std::move(formula));

    exists_f = RenameVar(std::move(exists_f).value());

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        exists_f.value().data.first,
        {!std::move(exists_f.value().data.second) && !std::move(g.value())})))};
  }

  return formula;
}

// (@ vx . F[vx]) and (@ vx . H[vx]) = (@ vx . F[vx] and H[vx])
inline parser::ImplicationFormula QuantifiersToCNF3(
    parser::ImplicationFormula formula) {
  // (@ vx . F[vx]) and (@ vx . H[vx]) = @ vx . F[vx] or H[vx]
  if (matcher::check::Conj(matcher::check::Forall(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::ForallFormula> forall_h;

    matcher::Conj(matcher::RefForall(forall_f), matcher::RefForall(forall_h))
        .match(std::move(formula));

    forall_h =
        RenameVar(std::move(forall_h.value()), forall_f.value().data.first);

    return {parser::MakeDisj(parser::MakeConj(
        parser::MakeForall(forall_f.value().data.first,
                           {!std::move(forall_f.value().data.second) &&
                            !std::move(forall_h.value().data.second)})))};
  }

  return formula;
}

// (? vx . F[vx]) or (? vx . H[vx]) = ? vx . F[vx] or H[vx]
inline parser::ImplicationFormula QuantifiersToCNF4(
    parser::ImplicationFormula formula) {
  // (? vx . F[vx]) or (? vx . H[vx]) = ? vx . F[vx] or H[vx]
  if (matcher::check::Disj(matcher::check::Exists(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    std::optional<parser::ExistsFormula> forall_h;

    matcher::Disj(matcher::RefExists(forall_f), matcher::RefExists(forall_h))
        .match(std::move(formula));

    forall_h =
        RenameVar(std::move(forall_h.value()), forall_f.value().data.first);

    return {parser::MakeDisj(parser::MakeConj(
        parser::MakeExists(forall_f.value().data.first,
                           {!std::move(forall_f.value().data.second) ||
                            !std::move(forall_h.value().data.second)})))};
  }

  return formula;
}

// (Q1 vx . F[vx]) and (Q2 vx . H[vx]) = Q1 vx . Q2 uc . F[vx] and H[uc]
inline parser::ImplicationFormula QuantifiersToCNF6(
    parser::ImplicationFormula formula) {
  // (? vx . F[vx]) and (? vx . H[vx]) = ? ucc . ? uc . F[ucc] and H[uc]
  if (matcher::check::Conj(matcher::check::Exists(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    std::optional<parser::ExistsFormula> forall_h;

    matcher::Conj(matcher::RefExists(forall_f), matcher::RefExists(forall_h))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f.value()));
    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeExists(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) &&
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  // (? vx . F[vx]) and (@ vx . H[vx]) = ? ucc . @ uc . F[ucc] and H[uc]
  if (matcher::check::Conj(matcher::check::Exists(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    std::optional<parser::ForallFormula> forall_h;

    matcher::Conj(matcher::RefExists(forall_f), matcher::RefForall(forall_h))
        .match(std::move(formula));

    forall_f = RenameVar(std::move(forall_f.value()));
    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeForall(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) &&
                                !std::move(forall_h.value().data.second)})))};

    return parser::ToFol(parser::MakeExists(forall_f.value().data.first,
                                            std::move(inner_forall)));
  }

  // (@ vx . F[vx]) and (? vx . H[vx]) = @ vx . ? uc . F[vx] and H[uc]
  if (matcher::check::Conj(matcher::check::Forall(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::ExistsFormula> forall_h;

    matcher::Conj(matcher::RefForall(forall_f), matcher::RefExists(forall_h))
        .match(std::move(formula));

    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeForall(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) &&
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  // (@ vx . F[vx]) and (@ vx . H[vx]) = @ vx . @ uc . F[vx] and H[uc]
  if (matcher::check::Conj(matcher::check::Forall(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::ForallFormula> forall_h;

    matcher::Conj(matcher::RefForall(forall_f), matcher::RefForall(forall_h))
        .match(std::move(formula));

    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeForall(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) &&
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  return formula;
}

// (Q1 vx . F[vx]) or (Q2 vx . H[vx]) = Q1 vx . Q2 uc . F[vx] or H[uc]
inline parser::ImplicationFormula QuantifiersToCNF5(
    parser::ImplicationFormula formula) {
  // (? vx . F[vx]) or (? vx . H[vx]) = ? vx . ? uc . F[vx] or H[uc]
  if (matcher::check::Disj(matcher::check::Exists(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    std::optional<parser::ExistsFormula> forall_h;

    matcher::Disj(matcher::RefExists(forall_f), matcher::RefExists(forall_h))
        .match(std::move(formula));

    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeExists(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) ||
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  // (? vx . F[vx]) or (@ vx . H[vx]) = ? vx . @ uc . F[vx] or H[uc]
  if (matcher::check::Disj(matcher::check::Exists(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    std::optional<parser::ForallFormula> forall_h;

    matcher::Disj(matcher::RefExists(forall_f), matcher::RefForall(forall_h))
        .match(std::move(formula));

    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeForall(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) ||
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeExists(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  // (@ vx . F[vx]) or (? vx . H[vx]) = @ vx . ? uc . F[vx] or H[uc]
  if (matcher::check::Disj(matcher::check::Forall(),
                           matcher::check::Exists())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::ExistsFormula> exists_h;

    matcher::Disj(matcher::RefForall(forall_f), matcher::RefExists(exists_h))
        .match(std::move(formula));

    exists_h = RenameVar(std::move(exists_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeExists(exists_h.value().data.first,
                               {!std::move(forall_f.value().data.second) ||
                                !std::move(exists_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  // (@ vx . F[vx]) or (@ vx . H[vx]) = @ vx . @ uc . F[vx] or H[uc]
  if (matcher::check::Disj(matcher::check::Forall(),
                           matcher::check::Forall())(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    std::optional<parser::ForallFormula> forall_h;

    matcher::Disj(matcher::RefForall(forall_f), matcher::RefForall(forall_h))
        .match(std::move(formula));

    forall_h = RenameVar(std::move(forall_h.value()));

    parser::ImplicationFormula inner_forall = {
        parser::MakeDisj(parser::MakeConj(
            parser::MakeForall(forall_h.value().data.first,
                               {!std::move(forall_f.value().data.second) ||
                                !std::move(forall_h.value().data.second)})))};

    return {parser::MakeDisj(parser::MakeConj(parser::MakeForall(
        forall_f.value().data.first, std::move(inner_forall))))};
  }

  return formula;
}

inline parser::ImplicationFormula NormalizeQuantifiersStep(
    parser::ImplicationFormula formula) {
  formula = QuantifiersToCNF3(std::move(formula));
  formula = QuantifiersToCNF4(std::move(formula));

  formula = QuantifiersToCNF5(std::move(formula));
  formula = QuantifiersToCNF6(std::move(formula));

  formula = QuantifiersToCNF1(std::move(formula));
  formula = QuantifiersToCNF2(std::move(formula));

  return formula;
}

inline parser::ImplicationFormula NormalizeQuantifiers(
    parser::ImplicationFormula formula) {
  formula = DeleteUselessBrackets(std::move(formula));

  if (IsNoQuantifiers(formula)) {
    return formula;
  }

  formula = NormalizeQuantifiersStep(std::move(formula));

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    matcher::RefForall(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeForall(
        forall_f.value().data.first,
        NormalizeQuantifiers(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    matcher::RefExists(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeExists(
        forall_f.value().data.first,
        NormalizeQuantifiers(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Disj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return NormalizeQuantifiers(
        parser::ToFol(!NormalizeQuantifiers(std::move(fol_lhs.value())) ||
                      !NormalizeQuantifiers(std::move(fol_rhs.value()))));
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Conj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return NormalizeQuantifiers(
        parser::ToFol(!NormalizeQuantifiers(std::move(fol_lhs.value())) &&
                      !NormalizeQuantifiers(std::move(fol_rhs.value()))));
  }

  return formula;
}

inline parser::ImplicationFormula RemoveImplication(
    parser::ImplicationFormula formula) {
  formula = DropAllOutBrackets(std::move(formula));
  formula = DropAllBracketsInNot(std::move(formula));
  // F -> G = (~ F) or (G)
  if (matcher::check::Impl(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> impl_f;
    std::optional<parser::ImplicationFormula> impl;

    matcher::Impl(matcher::RefImpl(impl_f), matcher::RefImpl(impl))
        .match(std::move(formula));

    parser::ImplicationFormula disj_t = RemoveImplication(
        {~!std::move(impl_f.value()) || !std::move(impl.value())});

    return disj_t;
  }

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    matcher::RefForall(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeForall(
        forall_f.value().data.first,
        RemoveImplication(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    matcher::RefExists(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeExists(
        forall_f.value().data.first,
        RemoveImplication(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Disj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return parser::ToFol(!RemoveImplication(std::move(fol_lhs.value())) ||
                         !RemoveImplication(std::move(fol_rhs.value())));
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Conj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return parser::ToFol(!RemoveImplication(std::move(fol_lhs.value())) &&
                         !RemoveImplication(std::move(fol_rhs.value())));
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::FolFormula> fol;
    matcher::Not(matcher::RefImpl(fol)).match(std::move(formula));
    return parser::ToFol(~!RemoveImplication(std::move(fol.value())));
  }

  return formula;
}

inline parser::ImplicationFormula MoveNegInner(
    parser::ImplicationFormula formula) {
  // ~(~F) = F
  if (matcher::check::Not(matcher::check::Not())(formula)) {
    std::optional<parser::ImplicationFormula> unary;
    matcher::Not(matcher::Not(matcher::RefImpl(unary)))
        .match(std::move(formula));

    return MoveNegInner(std::move(unary.value()));
  }

  // ~(F or G) = ~F and ~G
  if (matcher::check::Not(matcher::check::Disj(
          matcher::check::Anything(), matcher::check::Anything()))(formula)) {
    std::optional<parser::ImplicationFormula> impl_lhs;
    std::optional<parser::ImplicationFormula> impl_rhs;

    matcher::Not(matcher::Brackets(matcher::Disj(matcher::RefImpl(impl_lhs),
                                                 matcher::RefImpl(impl_rhs))))
        .match(std::move(formula));

    auto conj =
        MoveNegInner({!(matcher::UnaryToFol(~!std::move(impl_lhs.value()))) &&
                      !(matcher::UnaryToFol(~!std::move(impl_rhs.value())))});

    return conj;
  }

  // ~(F and G) = ~F or ~G
  if (matcher::check::Not(matcher::check::Conj(
          matcher::check::Anything(), matcher::check::Anything()))(formula)) {
    std::optional<parser::ImplicationFormula> impl_lhs;
    std::optional<parser::ImplicationFormula> impl_rhs;

    matcher::Not(matcher::Brackets(matcher::Conj(matcher::RefImpl(impl_lhs),
                                                 matcher::RefImpl(impl_rhs))))
        .match(std::move(formula));

    auto disj =
        MoveNegInner({!(matcher::UnaryToFol(~!std::move(impl_lhs.value()))) ||
                      !(matcher::UnaryToFol(~!std::move(impl_rhs.value())))});

    return disj;
  }

  // ~(@ vx . F) = ? vx . ~F
  if (matcher::check::Not(matcher::check::Forall())(formula)) {
    std::optional<std::string> var;
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::Forall(matcher::RefName(var), matcher::RefImpl(impl)))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var.value();

    return parser::Exists(
        std::move(var_v),
        MoveNegInner(matcher::UnaryToFol(~!std::move(impl.value()))));
  }

  // ~(? vx . F) = @ vx . ~F
  if (matcher::check::Not(matcher::check::Exists())(formula)) {
    std::optional<std::string> var;
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::Exists(matcher::RefName(var), matcher::RefImpl(impl)))
        .match(std::move(formula));

    lexer::Variable var_v;
    var_v.base() = var.value();

    return parser::ForAll(
        std::move(var_v),
        MoveNegInner(matcher::UnaryToFol(~!std::move(impl.value()))));
  }

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ForallFormula> forall_f;
    matcher::RefForall(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeForall(
        forall_f.value().data.first,
        MoveNegInner(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ExistsFormula> forall_f;
    matcher::RefExists(forall_f).match(std::move(formula));
    return parser::ToFol(parser::MakeExists(
        forall_f.value().data.first,
        MoveNegInner(std::move(forall_f.value().data.second))));
  }

  if (matcher::check::Disj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Disj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return parser::ToFol(!MoveNegInner(std::move(fol_lhs.value())) ||
                         !MoveNegInner(std::move(fol_rhs.value())));
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::FolFormula> fol_lhs;
    std::optional<parser::FolFormula> fol_rhs;
    matcher::Conj(matcher::RefImpl(fol_lhs), matcher::RefImpl(fol_rhs))
        .match(std::move(formula));

    return parser::ToFol(!MoveNegInner(std::move(fol_lhs.value())) &&
                         !MoveNegInner(std::move(fol_rhs.value())));
  }

  return formula;
}

inline parser::ImplicationFormula ToConjunctionNormalForm(
    parser::ImplicationFormula formula) {
  formula = DeleteUselessBrackets(std::move(formula));

  // Pred | ~Pred | [~]Pred or [~]Pred | [~]Pred and [~]Pred ::= <<end>>
  auto pred_checker = matcher::check::OrMatch(
      matcher::check::Pred(), matcher::check::Not(matcher::check::Pred()));
  if (pred_checker(formula) ||
      matcher::check::Disj(pred_checker, pred_checker)(formula) ||
      matcher::check::Conj(pred_checker, pred_checker)(formula)) {
    return formula;
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
    auto conj = !ToConjunctionNormalForm({!std::move(impl_f.value()) ||
                                          !std::move(impl_g.value())}) &&
                !ToConjunctionNormalForm(
                    {!std::move(impl_f_c) || !std::move(impl_h.value())});

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
    auto conj = !ToConjunctionNormalForm({!std::move(impl_f.value()) ||
                                          !std::move(impl_g.value())}) &&
                !ToConjunctionNormalForm(
                    {!std::move(impl_f_c) || !std::move(impl_h.value())});

    return ToConjunctionNormalForm({std::move(conj)});
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::ImplicationFormula> impl;
    matcher::Not(matcher::RefImpl(impl)).match(std::move(formula));

    return matcher::UnaryToFol(
        ~!ToConjunctionNormalForm(std::move(impl.value())));
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

    auto cnf = DeleteUselessBrackets(
        !ToConjunctionNormalForm(std::move(lhs.value())) ||
        !ToConjunctionNormalForm(std::move(rhs.value())));

    if (IsAllDisj(cnf)) {
      return {std::move(cnf)};
    } else {
      return ToConjunctionNormalForm({std::move(cnf)});
    }
  }

  if (matcher::check::Conj(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ImplicationFormula> lhs;
    std::optional<parser::ImplicationFormula> rhs;

    matcher::Conj(matcher::RefImpl(lhs), matcher::RefImpl(rhs))
        .match(std::move(formula));

    auto conj = DeleteUselessBrackets(
        !ToConjunctionNormalForm(std::move(lhs.value())) &&
        !ToConjunctionNormalForm(std::move(rhs.value())));

    return {std::move(conj)};
  }

  return formula;
}

inline parser::UnaryFormula DeleteUselessBrackets(
    parser::UnaryFormula formula) {
  if (matcher::check::Brackets(matcher::check::Unary())(formula)) {
    std::optional<parser::UnaryFormula> unary;
    matcher::RefUnary(unary).match(std::move(formula));

    return DeleteUselessBrackets(std::move(unary.value()));
  }

  if (matcher::check::Pred()(formula)) {
    std::optional<parser::PredicateFormula> pred;
    matcher::RefPred(pred).match(matcher::UnaryToFol(std::move(formula)));
    return {std::move(pred.value())};
  }

  if (matcher::check::Not()(formula)) {
    std::optional<parser::NotFormula> not_f;
    matcher::RefNot(not_f).match(std::move(formula));

    return ~DeleteUselessBrackets(std::move(*not_f.value().data));
  }

  if (matcher::check::Forall()(formula)) {
    std::optional<parser::ForallFormula> forall;
    matcher::RefForall(forall).match(std::move(formula));

    return parser::ForAll(
        std::move(forall->data.first),
        DeleteUselessBrackets(std::move(forall->data.second)));
  }

  if (matcher::check::Exists()(formula)) {
    std::optional<parser::ExistsFormula> exists;
    matcher::RefExists(exists).match(std::move(formula));

    return parser::Exists(
        std::move(exists->data.first),
        DeleteUselessBrackets(std::move(exists->data.second)));
  }

  return !DeleteUselessBrackets(matcher::UnaryToFol(std::move(formula)));
}

inline parser::ConjunctionFormula DeleteUselessBrackets(
    parser::ConjunctionFormula formula) {
  formula = DropAllOutBrackets({std::move(formula)});

  // (A) and (B) = A and B
  if (matcher::check::Conj(matcher::check::Unary(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::UnaryFormula> unary_a;
    std::optional<parser::ImplicationFormula> impl_b;

    matcher::Conj(matcher::RefUnary(unary_a), matcher::RefImpl(impl_b))
        .match({std::move(formula)});

    impl_b = DeleteUselessBrackets(std::move(impl_b.value()));
    auto lhs = DeleteUselessBrackets(std::move(unary_a.value()));

    // A >< C and D and ...
    if (matcher::check::Conj(matcher::check::Unary(),
                             matcher::check::Anything())(lhs)) {
      parser::ConjunctionFormula conj_lhs = parser::MakeConj(std::move(lhs));
      // B >< C and D and ...
      if (matcher::check::Conj(matcher::check::Anything(),
                               matcher::check::Anything())(impl_b.value())) {
        auto disj_b = Match(matcher::Conj(), std::move(impl_b.value()));
        return DeleteUselessBrackets(std::move(conj_lhs)) &&
               DeleteUselessBrackets(std::move(disj_b));
      }
      return DeleteUselessBrackets(std::move(conj_lhs)) &&
             !DeleteUselessBrackets(std::move(impl_b.value()));
    }

    // B >< C and D and ...
    if (matcher::check::Conj(matcher::check::Anything(),
                             matcher::check::Anything())(impl_b.value())) {
      auto disj_b = Match(matcher::Conj(), std::move(impl_b.value()));
      return DeleteUselessBrackets(std::move(lhs)) &&
             DeleteUselessBrackets(std::move(disj_b));
    }

    if (matcher::check::Unary()(impl_b.value())) {
      auto unary = Match(matcher::Unary(), std::move(impl_b.value()));
      return DeleteUselessBrackets(std::move(lhs)) &&
             DeleteUselessBrackets(std::move(unary));
    }

    return std::move(lhs) && !DeleteUselessBrackets(std::move(impl_b.value()));
  }

  if (matcher::check::Unary()(formula)) {
    std::optional<parser::UnaryFormula> unary;
    matcher::RefUnary(unary).match({std::move(formula)});

    return parser::MakeConj(DeleteUselessBrackets(std::move(unary.value())));
  }

  return formula;
}

inline parser::DisjunctionFormula DeleteUselessBrackets(
    parser::DisjunctionFormula formula) {
  formula = DropAllOutBrackets({std::move(formula)});

  // (A) or (B) = A or B
  if (matcher::check::Disj(matcher::check::Conj(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::ConjunctionFormula> conj_a;
    std::optional<parser::ImplicationFormula> impl_b;

    matcher::Disj(matcher::RefConj(conj_a), matcher::RefImpl(impl_b))
        .match({std::move(formula)});

    auto lhs = DeleteUselessBrackets(std::move(conj_a.value()));

    // A >< (C or D or ...)
    if (matcher::check::Disj(matcher::check::Anything(),
                             matcher::check::Anything())(lhs)) {
      parser::DisjunctionFormula disj_lhs = std::move(lhs);

      // B >< C or D or ...
      if (matcher::check::Disj(matcher::check::Anything(),
                               matcher::check::Anything())(impl_b.value())) {
        auto disj_b = Match(matcher::Disj(), std::move(impl_b.value()));
        return DeleteUselessBrackets(std::move(disj_lhs)) ||
               DeleteUselessBrackets(std::move(disj_b));
      }

      return DeleteUselessBrackets(
          DeleteUselessBrackets(std::move(disj_lhs)) ||
          !DeleteUselessBrackets(std::move(impl_b.value())));
    }

    // B >< C or D or ...
    if (matcher::check::Disj(matcher::check::Anything(),
                             matcher::check::Anything())(impl_b.value())) {
      auto disj_b = Match(matcher::Disj(), std::move(impl_b.value()));
      return DeleteUselessBrackets(std::move(lhs)) ||
             DeleteUselessBrackets(std::move(disj_b));
    }

    impl_b = DeleteUselessBrackets(std::move(impl_b.value()));
    if (matcher::check::Unary()(impl_b.value())) {
      auto unary = Match(matcher::Unary(), std::move(impl_b.value()));
      return DeleteUselessBrackets(std::move(lhs)) ||
             DeleteUselessBrackets(std::move(unary));
    }

    return std::move(lhs) || !DeleteUselessBrackets(std::move(impl_b.value()));
  }

  if (matcher::check::Conj()(formula)) {
    std::optional<parser::ConjunctionFormula> conj;
    matcher::RefConj(conj).match({std::move(formula)});

    return {DeleteUselessBrackets(std::move(conj.value()))};
  }

  return formula;
}

inline parser::FolFormula DeleteUselessBrackets(parser::FolFormula formula) {
  formula = DropAllOutBrackets(std::move(formula));
  formula = DropAllBracketsInNot(std::move(formula));

  // A -> (B) = A -> B
  if (matcher::check::Impl(matcher::check::Anything(),
                           matcher::check::Anything())(formula)) {
    std::optional<parser::DisjunctionFormula> impl_a;
    std::optional<parser::ImplicationFormula> impl_b;

    matcher::Impl(matcher::RefDisj(impl_a), matcher::RefImpl(impl_b))
        .match(std::move(formula));
    return DeleteUselessBrackets(std::move(impl_a.value())) >>=
           DeleteUselessBrackets(std::move(impl_b.value()));
  }

  if (matcher::check::Disj()(formula)) {
    std::optional<parser::DisjunctionFormula> disj;

    matcher::RefDisj(disj).match(std::move(formula));

    return {DeleteUselessBrackets(std::move(disj.value()))};
  }

  return DropAllOutBrackets(std::move(formula));
}

inline parser::FolFormula ToCNF(parser::FolFormula formula) {
  return DeleteUselessBrackets(ToConjunctionNormalForm(std::move(formula)));
}

inline parser::FolFormula Normalize(parser::FolFormula formula) {
  formula = RemoveImplication(std::move(formula));
  formula = MoveNegInner(std::move(formula));
  formula = NormalizeQuantifiers(std::move(formula));
  formula = ToCNF(std::move(formula));
  return formula;
}

}  // namespace fol::transform

