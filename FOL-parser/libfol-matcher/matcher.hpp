#pragma once

#include <functional>
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

template <typename NameMatcher, typename ImplMatcher>
struct ForallCompoundMatcher {
  bool match(parser::FolFormula o) {
    ForallMatcher forall_matcher;
    if (!forall_matcher.match(std::move(o))) {
      return false;
    }
    var.match(std::move(forall_matcher.formula->data.first));
    return matcher.match(std::move(forall_matcher.formula->data.second));
  }
  NameMatcher var;
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

struct NameMatcher {
  bool match(std::string str) {
    name = std::move(str);
    return true;
  }
  std::string name;
};

template <typename NameMatcher, typename TermListMatcher>
struct PredicateCompoundMatcher {
  bool match(parser::FolFormula o) {
    PredicateMatcher matcher;
    if (!matcher.match(std::move(o))) {
      return false;
    }

    predicate_name.match(std::move(matcher.formula->data.first));
    return term_list.match(std::move(matcher.formula->data.second));
  }
  NameMatcher predicate_name;
  TermListMatcher term_list{};
};

template <typename Type, typename MatchType, typename Matcher>
struct RefMatcher {
  bool match(MatchType o) {
    Matcher matcher{};
    if (!matcher.match(std::move(o))) {
      return false;
    }
    formula.get() = std::move(matcher.formula);
    return true;
  }
  std::reference_wrapper<std::optional<Type>> formula;
};

using RefNameMatcher = RefMatcher<std::string, std::string, NameMatcher>;

inline RefNameMatcher RefName(std::optional<std::string> &res) { return {res}; }

using RefImplMatcher = RefMatcher<parser::ImplicationFormula,
                                  parser::FolFormula, ImplicationMatcher>;

inline RefImplMatcher RefImpl(std::optional<parser::ImplicationFormula> &res) {
  return {res};
}

using RefDisjMatcher = RefMatcher<parser::DisjunctionFormula,
                                  parser::FolFormula, DisjunctionMatcher>;
inline RefDisjMatcher RefDisj(std::optional<parser::DisjunctionFormula> &res) {
  return {res};
}

using RefConjMatcher = RefMatcher<parser::ConjunctionFormula,
                                  parser::FolFormula, ConjunctionMatcher>;
inline RefConjMatcher RefConj(std::optional<parser::ConjunctionFormula> &res) {
  return {res};
}

using RefUnaryMatcher =
    RefMatcher<parser::UnaryFormula, parser::FolFormula, UnaryMatcher>;

inline RefUnaryMatcher RefUnary(std::optional<parser::UnaryFormula> &res) {
  return {res};
}

using RefNotMatcher =
    RefMatcher<parser::NotFormula, parser::FolFormula, NotMatcher>;
inline RefNotMatcher RefNot(std::optional<parser::NotFormula> &res) {
  return {res};
}

using RefForallMatcher =
    RefMatcher<parser::ForallFormula, parser::FolFormula, ForallMatcher>;
inline RefForallMatcher RefForall(std::optional<parser::ForallFormula> &res) {
  return {res};
}

using RefExistsMatcher =
    RefMatcher<parser::ExistsFormula, parser::FolFormula, ExistsMatcher>;
inline RefExistsMatcher RefExists(std::optional<parser::ExistsFormula> &res) {
  return {res};
}

using RefPredMatcher =
    RefMatcher<parser::PredicateFormula, parser::FolFormula, PredicateMatcher>;
inline RefPredMatcher RefPred(std::optional<parser::PredicateFormula> &res) {
  return {res};
}

using RefTerMatcher = RefMatcher<parser::Term, parser::TermList, TermMatcher>;
inline RefTerMatcher RefTerm(std::optional<parser::Term> &res) { return {res}; }

using RefConstantMatcher =
    RefMatcher<lexer::Constant, parser::TermList, ConstantMatcher>;
inline RefConstantMatcher RefConstant(std::optional<lexer::Constant> &res) {
  return {res};
}

using RefVariableMatcher =
    RefMatcher<lexer::Variable, parser::TermList, VariableMatcher>;
inline RefVariableMatcher RefVar(std::optional<lexer::Variable> &res) {
  return {res};
}

using RefFunctionMatcher =
    RefMatcher<parser::FunctionFormula, parser::TermList, FunctionMatcher>;
inline RefFunctionMatcher RefFunc(std::optional<parser::FunctionFormula> &res) {
  return {res};
}

inline ImplicationMatcher Impl() { return {}; }

template <typename FMatcher, typename SMatcher>
inline ImplicationCompoundMatcher<FMatcher, SMatcher> Impl(FMatcher &&lhs,
                                                           SMatcher &&rhs) {
  return {std::forward<FMatcher>(lhs), std::forward<SMatcher>(rhs)};
}

inline DisjunctionMatcher Disj() { return {}; }

template <typename FMatcher, typename SMatcher>
inline DisjunctionCompoundMatcher<FMatcher, SMatcher> Disj(FMatcher &&lhs,
                                                           SMatcher &&rhs) {
  return {std::forward<FMatcher>(lhs), std::forward<SMatcher>(rhs)};
}

inline ConjunctionMatcher Conj() { return {}; }

template <typename FMatcher, typename SMatcher>
inline ConjunctionCompoundMatcher<FMatcher, SMatcher> Conj(FMatcher &&lhs,
                                                           SMatcher &&rhs) {
  return {std::forward<SMatcher>(lhs), std::forward<SMatcher>(rhs)};
}

inline UnaryMatcher Unary() { return {}; }

inline ExistsMatcher Exists() { return {}; }

template <typename T>
inline ExistsCompoundMatcher<T> Exists(T &&o) {
  return {std::forward<T>(o)};
}

inline ForallMatcher Forall() { return {}; }

template <typename NMatcher, typename T>
inline ForallCompoundMatcher<NMatcher, T> Forall(NMatcher &&n, T &&o) {
  return {std::forward<NMatcher>(n), std::forward<T>(o)};
}

inline NameMatcher Name() { return {}; }

inline PredicateMatcher Pred() { return {}; }

template <typename NameMatcher, typename T>
inline PredicateCompoundMatcher<NameMatcher, T> Pred(NameMatcher &&m, T &&o) {
  return {std::forward<NameMatcher>(m), std::forward<T>(o)};
}

inline TermMatcher Term() { return {}; }

inline ConstantMatcher Const() { return {}; }

inline VariableMatcher Var() { return {}; }

inline FunctionMatcher Fun() { return {}; }

template <typename T>
inline FunctionCompoundMatcher<T> Fun(T &&o) {
  return {std::forward<T>(o)};
}

inline TermListMatcher TermList() { return {}; }

template <typename... Args>
inline TermListCompoundMatcher<Args...> TermList(Args...) {
  return {};
}

}  // namespace fol::matcher

