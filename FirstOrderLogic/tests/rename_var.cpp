#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-transform/replace.hpp>
#include <variant>
#include <vector>

using namespace fol;

TEST_CASE("rename var test", "[transform][fol]") {
  auto formula =
      parser::Parse(lexer::Tokenize("@ vx . pP(vx) -> pP(vy) and pP(vy)"));
  formula = transform::RenameVar(std::move(formula), "vx", "vy");
  REQUIRE(parser::ToString(formula) == "((@ vy . pP(vy)->pP(vy) and pP(vy)))");
  formula = transform::RenameVar(std::move(formula), "vx", "vy");
  REQUIRE(parser::ToString(formula) ==
          "(((@ vy . pP(vy)->pP(vy) and pP(vy))))");
  formula = transform::RenameVar(std::move(formula), "vy", "vz");
  REQUIRE(parser::ToString(formula) ==
          "((((@ vz . pP(vz)->pP(vz) and pP(vz)))))");
}

