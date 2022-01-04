#pragma once

#include <libfol-parser/parser/types.hpp>
#include <optional>
#include <variant>

namespace fol::matcher {

template <typename T, typename TFormula>
bool holds(const TFormula &lhs) {
  return std::holds_alternative<T>(lhs.data);
}

struct ImplicationMatcher {
  bool match(parser::FolFormula o) {
    formula = std::move(o);
    return true;
  }
  std::optional<parser::ImplicationFormula> formula{};
};

template <typename FMatcher, typename SMatcher>
struct ImplicationCompoundMatcher {
  bool match(parser::FolFormula o) {
    if (!std::holds_alternative<std::unique_ptr<
            std::pair<parser::DisjunctionFormula, parser::ImplicationFormula>>>(
            o.data)) {
      return false;
    }

    auto pair = std::get<std::unique_ptr<
        std::pair<parser::DisjunctionFormula, parser::ImplicationFormula>>>(
        std::move(o.data));
    auto disj = std::move(pair->first);
    auto impl = std::move(pair->second);

    return first.match(parser::FolFormula{std::move(disj)}) &&
           second.match(std::move(impl));
  }

  FMatcher first{};
  SMatcher second{};
};

struct DisjunctionMatcher {
  bool match(parser::FolFormula o) {
    if (!std::holds_alternative<parser::DisjunctionFormula>(o.data)) {
      return false;
    }

    formula = std::get<parser::DisjunctionFormula>(std::move(o.data));

    return true;
  }
  std::optional<parser::DisjunctionFormula> formula{};
};

inline parser::FolFormula DisjToFol(parser::DisjunctionFormula formula) {
  return parser::FolFormula{std::move(formula)};
}

inline parser::DisjunctionFormula DisjPrimeToDisj(
    parser::DisjunctionPrimeFormula disj_prime) {
  return parser::DisjunctionFormula{std::move(
      *std::get<std::unique_ptr<std::pair<parser::ConjunctionFormula,
                                          parser::DisjunctionPrimeFormula>>>(
          std::move(disj_prime.data)))};
}

template <typename FMatcher, typename SMatcher>
struct DisjunctionCompoundMatcher {
  bool match(parser::FolFormula o) {
    DisjunctionMatcher disj{};
    if (!disj.match(std::move(o))) {
      return false;
    }

    auto disj_f = std::move(disj.formula.value());

    std::optional<parser::DisjunctionFormula> disj_prime;

    if (std::holds_alternative<lexer::EPS>(disj_f.data.second.data)) {
      return false;
    }

    return first.match(DisjToFol(std::move(disj_f.data.first))) &&
           second.match(
               DisjToFol(DisjPrimeToDisj(std::move(disj_f.data.second))));
  }

  FMatcher first{};
  SMatcher second{};
};

struct ConjunctionMatcher {
  bool match(parser::FolFormula o) {
    DisjunctionMatcher matcher;
    if (!matcher.match(std::move(o))) {
      return false;
    }
    if (!std::holds_alternative<lexer::EPS>(
            matcher.formula->data.second.data)) {
      return false;
    }

    formula = std::move(matcher.formula->data.first);
    return true;
  }
  std::optional<parser::ConjunctionFormula> formula{};
};

inline parser::FolFormula UnaryToFol(parser::UnaryFormula formula) {
  return parser::FolFormula{parser::DisjunctionFormula{
      parser::ConjunctionFormula{std::make_unique<
          std::pair<parser::UnaryFormula, parser::ConjuctionPrimeFormula>>(
          std::move(formula), parser::ConjuctionPrimeFormula{lexer::EPS{}})},
      parser::DisjunctionPrimeFormula{lexer::EPS{}}}};
}

inline parser::ConjunctionFormula ConjPrimeToConj(
    parser::ConjuctionPrimeFormula formula) {
  return parser::ConjunctionFormula{std::move(
      std::get<std::unique_ptr<
          std::pair<parser::UnaryFormula, parser::ConjuctionPrimeFormula>>>(
          formula.data))};
}

template <typename FMatcher, typename SMatcher>
struct ConjunctionCompoundMatcher {
  bool match(parser::FolFormula o) {
    ConjunctionMatcher conj_matcher;
    if (!conj_matcher.match(std::move(o))) {
      return false;
    }

    if (std::holds_alternative<lexer::EPS>(
            conj_matcher.formula->data->second.data)) {
      return false;
    }

    auto lhs_unary = std::move(conj_matcher.formula->data->first);
    auto rhs_conj = std::move(conj_matcher.formula->data->second);

    return first.match(UnaryToFol(std::move(lhs_unary))) &&
           second.match(DisjToFol(ConjPrimeToConj(std::move(rhs_conj))));
  }
  FMatcher first{};
  SMatcher second{};
};

struct UnaryMatcher {
  bool match(parser::FolFormula o) {
    DisjunctionMatcher disj_matcher;
    if (!disj_matcher.match(std::move(o))) {
      return false;
    }

    ConjunctionMatcher conj_matcher;
    if (!conj_matcher.match(
            DisjToFol(std::move(disj_matcher.formula.value())))) {
      return false;
    }

    auto conj = std::move(conj_matcher.formula.value());

    if (!std::holds_alternative<lexer::EPS>(conj.data->second.data)) {
      return false;
    }

    formula = std::move(conj.data->first);

    if (std::holds_alternative<parser::BracketFormula>(formula->data)) {
      return match(
          std::get<parser::BracketFormula>(std::move(formula->data)).data);
    }

    return true;
  }
  std::optional<parser::UnaryFormula> formula{};
};

struct ExistsMatcher {
  bool match(parser::FolFormula o) {
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

  std::optional<parser::ExistsFormula> formula{};
};

template <typename ImplMatcher>
struct ExistsCompoundMatcher {
  bool match(parser::FolFormula o) {
    ExistsMatcher exists_matcher;
    if (!exists_matcher.match(std::move(o))) {
      return false;
    }
    var = std::move(exists_matcher.formula->data.first);
    return matcher.match(std::move(exists_matcher.formula->data.second));
  }
  std::optional<lexer::Variable> var;
  ImplMatcher matcher;
};

struct NotMatcher {
  bool match(parser::FolFormula o) {
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

  std::optional<parser::NotFormula> formula{};
};

template <typename ImplMatcher>
struct NotCompoundMatcher {
  bool match(parser::FolFormula o) {
    NotMatcher forall_matcher;
    if (!forall_matcher.match(std::move(o))) {
      return false;
    }
    return matcher.match(std::move(forall_matcher.formula->data));
  }
  ImplMatcher matcher;
};

struct ForallMatcher {
  bool match(parser::FolFormula o) {
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

  std::optional<parser::ForallFormula> formula{};
};

template <typename ImplMatcher>
struct ForallCompoundMatcher {
  bool match(parser::FolFormula o) {
    ForallMatcher forall_matcher;
    if (!forall_matcher.match(std::move(o))) {
      return false;
    }
    var = std::move(forall_matcher.formula->data.first);
    return matcher.match(std::move(forall_matcher.formula->data.second));
  }
  std::optional<lexer::Variable> var;
  ImplMatcher matcher;
};

struct PredicateMatcher {
  bool match(parser::FolFormula o) {
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
  std::optional<parser::PredicateFormula> formula{};
};

struct TermMatcher {
  bool match(parser::TermList term_list) {
    if (!std::holds_alternative<lexer::EPS>(term_list.data.second.data)) {
      return false;
    }
    term = std::move(term_list.data.first);
    return true;
  }
  std::optional<parser::Term> term;
};

struct TermListMatcher {
  bool match(parser::TermList o) {
    term_list = std::move(o);
    return true;
  }
  std::optional<parser::TermList> term_list;
};

template <typename T, typename Label>
struct Anotate : T {};

template <typename Head, typename... Tail>
struct TermListCompoundMatcher
    : Anotate<Head, struct HeadLabel>,
      Anotate<TermListCompoundMatcher<Tail...>, struct TailLabel> {
  bool match(parser::TermList o) {
    return head().match(std::move(o.data.first)) &&
           tail().match(std::move(o.data.second));
  }

  Head &head() { return *this; }
  const Head &head() const { return *this; }

  TermListCompoundMatcher<Tail...> &tail() { return *this; }
  const TermListCompoundMatcher<Tail...> &tail() const { return *this; }
};

template <typename Head>
struct TermListCompoundMatcher<Head> : Anotate<Head, struct HeadLabel> {
  bool match(parser::TermList o) { return head().match(std::move(o)); }

  Head &head() { return *this; }
  const Head &head() const { return *this; }
};

struct VariableMatcher {
  bool match(parser::TermList o) {
    TermMatcher matcher;
    if (!matcher.match(std::move(o))) {
      return false;
    }

    if (!std::holds_alternative<lexer::Variable>(matcher.term->data)) {
      return false;
    }

    variable = std::get<lexer::Variable>(std::move(matcher.term->data));
    return true;
  }
  std::optional<lexer::Variable> variable;
};

struct FunctionMatcher {
  bool match(parser::TermList o) {
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
  std::optional<parser::FunctionFormula> function;
};

template <typename TermListMatcher>
struct FunctionCompoundMatcher {
  bool match(parser::TermList o) {
    FunctionMatcher matcher;
    if (!matcher.match(std::move(o))) {
      return false;
    }

    function_name = std::move(matcher.function->data->first);

    return term_list.match(std::move(matcher.function->data->second));
  }
  std::optional<lexer::Function> function_name;
  TermListMatcher term_list;
};

struct ConstantMatcher {
  bool match(parser::TermList o) {
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
  std::optional<lexer::Constant> constant;
};

template <typename TermListMatcher>
struct PredicateCompoundMatcher {
  bool match(parser::FolFormula o) {
    PredicateMatcher matcher;
    if (!matcher.match(std::move(o))) {
      return false;
    }

    predicate_name = std::move(matcher.formula->data.first);
    return term_list.match(std::move(matcher.formula->data.second));
  }
  std::optional<lexer::Predicate> predicate_name;
  TermListMatcher term_list{};
};

inline ImplicationMatcher Impl() { return {}; }

template <typename FMatcher, typename SMatcher>
inline ImplicationCompoundMatcher<FMatcher, SMatcher> Impl(FMatcher, SMatcher) {
  return {};
}

inline DisjunctionMatcher Disj() { return {}; }

template <typename FMatcher, typename SMatcher>
inline DisjunctionCompoundMatcher<FMatcher, SMatcher> Disj(FMatcher, SMatcher) {
  return {};
}

inline ConjunctionMatcher Conj() { return {}; }

template <typename FMatcher, typename SMatcher>
inline ConjunctionCompoundMatcher<FMatcher, SMatcher> Conj(FMatcher, SMatcher) {
  return {};
}

inline UnaryMatcher Unary() { return {}; }

inline ExistsMatcher Exists() { return {}; }

template <typename T>
inline ExistsCompoundMatcher<T> Exists(T) {
  return {};
}

inline ForallMatcher Forall() { return {}; }

template <typename T>
inline ForallCompoundMatcher<T> Forall(T) {
  return {};
}

inline PredicateMatcher Pred() { return {}; }

template <typename T>
inline PredicateCompoundMatcher<T> Pred(T) {
  return {};
}

inline TermMatcher Term() { return {}; }

inline ConstantMatcher Const() { return {}; }

inline VariableMatcher Var() { return {}; }

inline FunctionMatcher Fun() { return {}; }

template <typename T>
inline FunctionCompoundMatcher<T> Fun(T) {
  return {};
}

inline TermListMatcher TermList() { return {}; }

template <typename... Args>
inline TermListCompoundMatcher<Args...> TermList(Args...) {
  return {};
}

}  // namespace fol::matcher

