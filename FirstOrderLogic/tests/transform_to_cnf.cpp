#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/normalization.hpp>
#include <variant>
#include <vector>
using namespace fol;
using namespace fol::parser;
using namespace fol::matcher;

TEST_CASE("test basic patterns", "[transform][fol]") {
  auto fol = Parse(lexer::Tokenize("pP1(vx)"));
  REQUIRE(check::Pred()(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("~(~pP1(vx))"));
  REQUIRE(check::Pred()(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("pP1(vx) -> pP2(vx)"));
  REQUIRE(check::Disj(check::Not(check::Pred()),
                      check::Pred())(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("~(pP1(vx) or pP2(vx))"));
  auto cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Conj(check::Not(check::Pred()),
                      check::Not(check::Pred()))(cnf_fol));

  fol = Parse(lexer::Tokenize("~(pP1(vx) and pP2(vx))"));
  REQUIRE(check::Disj(check::Not(check::Pred()), check::Not(check::Pred()))(
      transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("~(@ vx . pP1(vx))"));
  cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Exists(check::Not(check::Pred()))(cnf_fol));

  fol = Parse(lexer::Tokenize("~(? vx . pP1(vx))"));
  cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Forall(check::Not(check::Pred()))(cnf_fol));
}

TEST_CASE("test basic patterns with brackets", "[transform][fol]") {
  auto fol = Parse(lexer::Tokenize("(pP1(vx))"));
  REQUIRE(check::Pred()(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("~((pP1(vx) or pP2(vx)) and pP3(vx))"));
  REQUIRE(check::Disj(
      check::Conj(check::Not(check::Pred()), check::Not(check::Pred())),
      check::Not(check::Pred()))(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("(~(~pP1(vx)))"));
  REQUIRE(check::Pred()(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("(pP1(vx) -> pP2(vx))"));
  REQUIRE(check::Disj(check::Not(check::Pred()),
                      check::Pred())(transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("(~(pP1(vx) or pP2(vx)))"));
  auto cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Conj(check::Not(check::Pred()),
                      check::Not(check::Pred()))(cnf_fol));

  fol = Parse(lexer::Tokenize("(~(pP1(vx) and pP2(vx)))"));
  REQUIRE(check::Disj(check::Not(check::Pred()), check::Not(check::Pred()))(
      transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("(~(@ vx . pP1(vx)))"));
  cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Exists(check::Not(check::Pred()))(cnf_fol));

  fol = Parse(lexer::Tokenize("(~(? vx . pP1(vx)))"));
  cnf_fol = transform::ToCNF(std::move(fol));
  REQUIRE(check::Forall(check::Not(check::Pred()))(cnf_fol));
}

TEST_CASE("test complex patterns", "[transform][fol]") {
  auto fol = Parse(lexer::Tokenize("p3(v3) and p4(v4) or p1(v1)"));
  REQUIRE(check::Conj(check::Disj(check::Pred(), check::Pred()),
                      check::Disj(check::Pred(), check::Pred()))(
      transform::ToCNF(std::move(fol))));

  fol = Parse(lexer::Tokenize("p3(v3) or p4(v4) and p1(v1)"));
  REQUIRE(check::Conj(check::Disj(check::Pred(), check::Pred()),
                      check::Disj(check::Pred(), check::Pred()))(
      transform::ToCNF(std::move(fol))));

  fol = Parse(
      lexer::Tokenize("(p1(v1)->p2(v2)) and ((~p2(v2)->p3(v3))->~p1(v1))"));
  std::cout << transform::ToCNF(std::move(fol)) << std::endl;
}

