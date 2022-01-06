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
}

