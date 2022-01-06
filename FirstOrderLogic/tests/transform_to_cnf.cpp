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

