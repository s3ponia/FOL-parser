#include <libfol-matcher/matcher.hpp>

namespace fol::matcher {
bool ImplicationMatcher::match(parser::FolFormula o) {
  formula = std::move(o);
  return true;
}

bool DisjunctionMatcher::match(parser::FolFormula o) {
  if (!std::holds_alternative<parser::DisjunctionFormula>(o.data)) {
    return false;
  }

  formula = std::get<parser::DisjunctionFormula>(std::move(o.data));

  return true;
}

bool ConjunctionMatcher::match(parser::FolFormula o) {
  DisjunctionMatcher matcher;
  if (!matcher.match(std::move(o))) {
    return false;
  }

  if (!std::holds_alternative<lexer::EPS>(matcher.formula->data.second.data)) {
    return false;
  }

  formula = std::move(matcher.formula->data.first);
  return true;
}

bool UnaryMatcher::match(parser::FolFormula o) {
  ConjunctionMatcher conj_matcher;
  if (!conj_matcher.match(std::move(o))) {
    return false;
  }

  auto conj = std::move(conj_matcher.formula.value());

  if (!std::holds_alternative<lexer::EPS>(conj.data->second.data)) {
    return false;
  }

  formula = std::move(conj.data->first);

  if (check::Brackets(check::Unary())(formula.value())) {
    return match(
        std::get<parser::BracketFormula>(std::move(formula->data)).data);
  }

  return true;
}

bool BracketsMatcher::match(parser::FolFormula o) {
  if (!std::holds_alternative<parser::DisjunctionFormula>(o.data)) {
    formula = parser::MakeBrackets(std::move(o));
    return true;
  }

  DisjunctionMatcher disj_matcher;
  if (!disj_matcher.match(std::move(o))) {
    std::terminate();
  }

  if (!std::holds_alternative<lexer::EPS>(
          disj_matcher.formula->data.second.data)) {
    formula = parser::MakeBrackets({std::move(disj_matcher.formula.value())});
    return true;
  }

  ConjunctionMatcher conj_matcher;
  if (!conj_matcher.match({std::move(disj_matcher.formula.value())})) {
    std::terminate();
  }

  auto conj = std::move(conj_matcher.formula.value());

  if (!std::holds_alternative<lexer::EPS>(conj.data->second.data)) {
    formula = parser::MakeBrackets({parser::MakeDisj(std::move(conj))});
    return true;
  }

  auto formula_t = std::move(conj.data->first);

  if (!std::holds_alternative<parser::BracketFormula>(formula_t.data)) {
    formula = parser::MakeBrackets(std::move(formula_t));
  } else {
    return match(std::move(
        std::get<parser::BracketFormula>(std::move(formula_t.data)).data));
  }

  return true;
}

bool ExistsMatcher::match(parser::FolFormula o) {
  UnaryMatcher unary_matcher;
  if (!unary_matcher.match(std::move(o))) {
    return false;
  }

  auto unary = std::move(unary_matcher.formula.value());

  if (!std::holds_alternative<parser::ExistsFormula>(unary.data)) {
    return false;
  }

  formula = std::get<parser::ExistsFormula>(std::move(unary.data));
  return true;
}

bool NotMatcher::match(parser::FolFormula o) {
  UnaryMatcher unary_matcher;
  if (!unary_matcher.match(std::move(o))) {
    return false;
  }

  auto unary = std::move(unary_matcher.formula.value());

  if (!std::holds_alternative<parser::NotFormula>(unary.data)) {
    return false;
  }

  formula = std::get<parser::NotFormula>(std::move(unary.data));

  return true;
}

bool ForallMatcher::match(parser::FolFormula o) {
  UnaryMatcher unary_matcher;
  if (!unary_matcher.match(std::move(o))) {
    return false;
  }

  auto unary = std::move(unary_matcher.formula.value());

  if (!std::holds_alternative<parser::ForallFormula>(unary.data)) {
    return false;
  }

  formula = std::get<parser::ForallFormula>(std::move(unary.data));
  return true;
}

bool PredicateMatcher::match(parser::FolFormula o) {
  UnaryMatcher unary_matcher;
  if (!unary_matcher.match(std::move(o))) {
    return false;
  }

  if (!std::holds_alternative<parser::PredicateFormula>(
          unary_matcher.formula->data)) {
    return false;
  }

  formula = std::move(
      std::get<parser::PredicateFormula>(unary_matcher.formula->data));
  return true;
}

bool TermMatcher::match(parser::TermList term_list) {
  if (!std::holds_alternative<lexer::EPS>(term_list.data.second.data)) {
    return false;
  }
  term = std::move(term_list.data.first);
  return true;
}

bool TermListMatcher::match(parser::TermList o) {
  term_list = std::move(o);
  return true;
}

bool VariableMatcher::match(parser::TermList o) {
  TermMatcher matcher;
  if (!matcher.match(std::move(o))) {
    return false;
  }

  if (!std::holds_alternative<lexer::Variable>(matcher.term->data)) {
    return false;
  }

  formula = std::get<lexer::Variable>(std::move(matcher.term->data));
  return true;
}

bool FunctionMatcher::match(parser::TermList o) {
  TermMatcher matcher;
  if (!matcher.match(std::move(o))) {
    return false;
  }

  if (!std::holds_alternative<parser::FunctionFormula>(matcher.term->data)) {
    return false;
  }

  function = std::get<parser::FunctionFormula>(std::move(matcher.term->data));
  return true;
}

bool ConstantMatcher::match(parser::TermList o) {
  TermMatcher matcher;
  if (!matcher.match(std::move(o))) {
    return false;
  }

  if (!std::holds_alternative<lexer::Constant>(matcher.term->data)) {
    return false;
  }

  constant = std::get<lexer::Constant>(std::move(matcher.term->data));
  return true;
}

bool NameMatcher::match(std::string str) {
  formula = std::move(str);
  return true;
}
}  // namespace fol::matcher
