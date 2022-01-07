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

  fol_formula =
      Parse(lexer::Tokenize("((~(p3(v3))) and (~(p4(v4)))) or (~p1(v1))"));
  std::cout << fol_formula << std::endl;
  REQUIRE(Disj(Brackets(Conj(Impl(), Impl())), Impl())
              .match(std::move(fol_formula)));
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
  REQUIRE(check::Forall()(Parse(lexer::Tokenize("@ vx . pP1(vx)"))));
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
  fol_formula =
      Parse(lexer::Tokenize("? vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)"));
  REQUIRE(check::Exists(check::Impl(
      check::Pred(), check::Impl(check::Pred(), check::Impl())))(fol_formula));

  fol_formula = Parse(lexer::Tokenize("~ (pP1(vx)->pP2(vx))"));
  REQUIRE(check::Impl()(fol_formula));
  REQUIRE(!check::Impl(check::Pred(), check::Pred())(fol_formula));
  REQUIRE(check::Not(check::Impl(check::Pred(), check::Pred()))(fol_formula));
}

TEST_CASE("test not not match", "[checker][matcher][fol]") {
  auto fol = Parse(lexer::Tokenize("~(~pP(vx))"));
  REQUIRE(!(check::Pred()(fol)));
  REQUIRE(check::Not(check::Not())(fol));

  fol = Parse(lexer::Tokenize("~(pP(vx))"));
  REQUIRE(!check::Not(check::Not())(fol));
}

TEST_CASE("test De Morgan's laws", "[checker][matcher][fol]") {
  auto fol = Parse(lexer::Tokenize("~((pF(vx) or pG(vx)) or pS(vx))"));
  REQUIRE(!(check::Disj(check::Anything(), check::Pred())(fol)));
  REQUIRE(check::Not(check::Disj(
      check::Disj(check::Anything(), check::Anything()), check::Pred()))(fol));

  fol = Parse(lexer::Tokenize("pP1(vx) or pP3(vx)"));
  REQUIRE(check::Disj(check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("pP1(vx) or pP2(vx) or pP3(vx)"));
  REQUIRE(check::Disj(check::Anything(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Pred(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("pP1(vx) and pP2(vx) and pP3(vx)"));
  REQUIRE(check::Conj(check::Anything(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("~(pF(vx) and pG(vx))"));
  REQUIRE(!(check::Disj(check::Anything(), check::Anything())(fol)));
  REQUIRE(!(check::Conj(check::Anything(), check::Anything())(fol)));
  REQUIRE(check::Not(check::Conj(check::Anything(), check::Anything()))(fol));

  fol = Parse(lexer::Tokenize("pP1(vx) and pP2(vx) and pP3(vx) and pP4(vx)"));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Pred(),
                      check::Pred())(fol));
  REQUIRE(!check::Conj(check::Pred(), check::Pred(), check::Pred())(fol));
}

TEST_CASE("test forall and exists patterns", "[checker][matcher][fol]") {
  auto fol = Parse(lexer::Tokenize("~(@ vx . pP(vx))"));
  REQUIRE(!matcher::check::Forall()(fol));
  REQUIRE(matcher::check::Not()(fol));
  REQUIRE(matcher::check::Not(matcher::check::Forall())(fol));

  fol = Parse(lexer::Tokenize("~(? vx . pP(vx))"));
  REQUIRE(!matcher::check::Forall()(fol));
  REQUIRE(!matcher::check::Exists()(fol));
  REQUIRE(matcher::check::Not()(fol));
  REQUIRE(matcher::check::Not(matcher::check::Exists())(fol));
}

TEST_CASE(
    "test brackets"
    "[checker][matcher][fol]") {
  auto fol = Parse(lexer::Tokenize("(~(@ vx . pP(vx)))"));
  REQUIRE(!matcher::check::Forall()(fol));
  REQUIRE(matcher::check::Not()(fol));
  REQUIRE(matcher::check::Not(matcher::check::Forall())(fol));

  fol = Parse(lexer::Tokenize("(~(? vx . pP(vx)))"));
  REQUIRE(!matcher::check::Forall()(fol));
  REQUIRE(!matcher::check::Exists()(fol));
  REQUIRE(matcher::check::Not()(fol));
  REQUIRE(matcher::check::Not(matcher::check::Exists())(fol));

  fol = Parse(lexer::Tokenize("(~((pF(vx) or pG(vx)) or pS(vx)))"));
  REQUIRE(!(check::Disj(check::Anything(), check::Pred())(fol)));
  REQUIRE(check::Not(check::Disj(
      check::Disj(check::Anything(), check::Anything()), check::Pred()))(fol));

  fol = Parse(lexer::Tokenize("(pP1(vx) or pP3(vx))"));
  REQUIRE(check::Disj(check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("(pP1(vx) or pP2(vx) or pP3(vx))"));
  REQUIRE(check::Disj(check::Anything(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Pred(), check::Anything())(fol));
  REQUIRE(check::Disj(check::Pred(), check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("(pP1(vx) and pP2(vx) and pP3(vx))"));
  REQUIRE(check::Conj(check::Anything(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Anything())(fol));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Pred())(fol));

  fol = Parse(lexer::Tokenize("(~(pF(vx) and pG(vx)))"));
  REQUIRE(!(check::Disj(check::Anything(), check::Anything())(fol)));
  REQUIRE(!(check::Conj(check::Anything(), check::Anything())(fol)));
  REQUIRE(check::Not(check::Conj(check::Anything(), check::Anything()))(fol));

  fol = Parse(lexer::Tokenize("(pP1(vx) and pP2(vx) and pP3(vx) and pP4(vx))"));
  REQUIRE(check::Conj(check::Pred(), check::Pred(), check::Pred(),
                      check::Pred())(fol));
  REQUIRE(!check::Conj(check::Pred(), check::Pred(), check::Pred())(fol));
}

TEST_CASE("test brackets matching", "[checker][matcher][fol]") {
  auto fol = Parse(lexer::Tokenize("~(pP1(vx))"));
  REQUIRE(!check::Brackets()(fol));

  fol = Parse(lexer::Tokenize("pP1(vx)"));
  REQUIRE(!check::Brackets()(fol));

  fol = Parse(lexer::Tokenize("(pP1(vx))"));
  REQUIRE(check::Brackets()(fol));
  REQUIRE(!check::Conj(check::Anything(), check::Anything())(fol));
}

