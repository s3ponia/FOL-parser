#include <algorithm>
#include <catch2/catch.hpp>
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
  ForallCompoundMatcher<ImplicationCompoundMatcher<
      PredicateCompoundMatcher<VariableMatcher>, ImplicationMatcher>>
      matcher;
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(matcher.var == "vx");
  REQUIRE(matcher.matcher.first.predicate_name == "pP1");
  REQUIRE(matcher.matcher.first.term_list.variable == "vx");
  REQUIRE(ToString(matcher.matcher.second.formula.value()) ==
          "pP2(vx)->pP3(vx)->pP4(vx)");
}

TEST_CASE("test compound matchers with factory functions", "[matcher][fol]") {
  auto fol_formula =
      Parse(lexer::Tokenize("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  auto matcher =
      Forall(Impl(Pred(Var()), Impl(Pred(Var()), Impl(Pred(Var()), Impl()))));
  REQUIRE(matcher.match(std::move(fol_formula)));
  REQUIRE(matcher.var == "vx");
  REQUIRE(matcher.matcher.first.predicate_name == "pP1");
  REQUIRE(matcher.matcher.first.term_list.variable == "vx");
}

