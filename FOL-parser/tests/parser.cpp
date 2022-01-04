#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-parser/lexer.hpp>
#include <libfol-parser/parser.hpp>
#include <variant>
#include <vector>
using namespace fol::lexer;
using namespace fol::parser;

TEST_CASE("simple std::cout output", "[parser][fol]") {
  std::cout << Parse(Tokenize("@ va . (pP1(c1) and pP2(c3))")) << std::endl;
  std::cout << Parse(
                   Tokenize("@ vx . pP1(vx) -> pP2(vx) -> pP3(vx) -> pP4(vx)"))
            << std::endl;
  std::cout << Parse(Tokenize("@ vx . pHuman(vx) -> pMortal(vx)")) << std::endl;
  std::cout << Parse(Tokenize("((@ vx . (pHuman(vx) -> pMortal(vx)) and "
                              "pHuman(cSocrates)) -> pMortal(cSocrates))"))
            << std::endl;
}

