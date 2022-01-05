#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <variant>
#include <vector>
using namespace fol;
using namespace fol::parser;
using namespace fol::matcher;

TEST_CASE("test implication compound matcher", "[matcher][fol]") {
  auto fol_formula = Parse(lexer::Tokenize("(@ vx . pP1(vx)) -> pP2(vy)"));
  ImplicationCompoundMatcher<ForallMatcher, ImplicationMatcher> matcher;
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(ToString(matcher.first.formula.value()) == "@ vx . pP1(vx)");
  REQUIRE(ToString(matcher.second.formula.value()) == "pP2(vy)");
}

TEST_CASE("test disjunction compound matcher", "[matcher][fol]") {
  auto fol_formula = Parse(lexer::Tokenize("(@ vx . pP1(vx)) or pP2(vy)"));
  DisjunctionCompoundMatcher<ForallMatcher, ImplicationMatcher> matcher;
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(ToString(matcher.first.formula.value()) == "@ vx . pP1(vx)");
  REQUIRE(ToString(matcher.second.formula.value()) == "pP2(vy)");
}

TEST_CASE("test compound matchers", "[matcher][fol]") {
  auto fol_formula =
      Parse(lexer::Tokenize("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  ForallCompoundMatcher<
      NameMatcher, ImplicationCompoundMatcher<
                       PredicateCompoundMatcher<NameMatcher, VariableMatcher>,
                       ImplicationMatcher>>
      matcher;
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(matcher.var.formula == "vx");
  REQUIRE(matcher.matcher.first.predicate_name.formula == "pP1");
  REQUIRE(matcher.matcher.first.term_list.formula == "vx");
  REQUIRE(ToString(matcher.matcher.second.formula.value()) ==
          "pP2(vx)->pP3(vx)->pP4(vx)");
}

TEST_CASE("test compound matchers with factory functions", "[matcher][fol]") {
  auto fol_formula =
      Parse(lexer::Tokenize("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  auto matcher = Forall(Name(), Impl(Pred(Name(), Var()),
                                     Impl(Pred(Name(), Var()),
                                          Impl(Pred(Name(), Var()), Impl()))));
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(matcher.var.formula == "vx");
  REQUIRE(matcher.matcher.first.predicate_name.formula == "pP1");
  REQUIRE(matcher.matcher.first.term_list.formula == "vx");
}

TEST_CASE("test compound matchers with ref factory functions",
          "[matcher][fol]") {
  auto fol_formula =
      Parse(lexer::Tokenize("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  std::optional<std::string> name;
  std::optional<std::string> fst_pred_name;
  std::optional<lexer::Variable> var;
  auto matcher = Forall(
      RefName(name),
      Impl(Pred(RefName(fst_pred_name), RefVar(var)),
           Impl(Pred(Name(), Var()), Impl(Pred(Name(), Var()), Impl()))));
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(name.value() == "vx");
  REQUIRE(fst_pred_name.value() == "pP1");
  REQUIRE(var.value() == "vx");
}

TEST_CASE("test check simple matchers", "[checker][matcher][fol]") {
  REQUIRE(check::Disj().check(Parse(lexer::Tokenize("@ vx . pP1(vx)"))));
  REQUIRE(check::Impl(check::Forall())
              .check(Parse(lexer::Tokenize("@ vx . pP1(vx)"))));
  REQUIRE(check::Exists().check(Parse(lexer::Tokenize("? vx . pP1(vx)"))));
  REQUIRE(check::Pred().check(Parse(lexer::Tokenize("pP1(vx)"))));
  REQUIRE(check::Impl().check(Parse(lexer::Tokenize("pP2(vx) -> pP1(vx)"))));
}

TEST_CASE("test check compound matchers", "[checker][matcher][fol]") {
  auto fol_formula =
      Parse(lexer::Tokenize("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  auto checker = check::Forall(
      check::Impl(check::Pred(), check::Impl(check::Pred(), check::Impl())));
  REQUIRE(checker.check(fol_formula));
}

