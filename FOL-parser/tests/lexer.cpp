#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-parser/lexer.hpp>
#include <variant>
#include <vector>
using namespace fol::lexer;

TEST_CASE("tokenize single constant", "[lexer][fol]") {
  auto generator = Tokenize("cConst1");
  auto result = *generator.begin();
  REQUIRE(std::holds_alternative<Constant>(result));
  REQUIRE(std::get<Constant>(result) == Constant{"cConst1"});

  generator = Tokenize("cConst2");
  result = *generator.begin();
  REQUIRE(std::holds_alternative<Constant>(result));
  REQUIRE(std::get<Constant>(result) == Constant{"cConst2"});

  generator = Tokenize("cSDAbcxznbmcnbhajhskdhqwWEH2139us9aduD'asd'''");
  result = *generator.begin();
  REQUIRE(std::holds_alternative<Constant>(result));
  REQUIRE(std::get<Constant>(result) ==
          Constant{"cSDAbcxznbmcnbhajhskdhqwWEH2139us9aduD'asd'''"});
}

TEST_CASE("tokenize sequence", "[lexer][fol]") {
  auto generator =
      Tokenize("cConst and pPredicate or vVariable @ fFunction ? fBar . -> ,");
  auto vec = std::vector<Lexeme>{Constant{"cConst"},
                                 And{},
                                 Predicate{"pPredicate"},
                                 Or{},
                                 Variable{"vVariable"},
                                 Forall{},
                                 Function{"fFunction"},
                                 Exists{},
                                 Function{"fBar"},
                                 Dot{},
                                 Implies{},
                                 Coma{},
                                 EPS{}};

  REQUIRE(std::equal(vec.begin(), vec.end(), generator.begin()));
}

