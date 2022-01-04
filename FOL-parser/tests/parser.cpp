#include <algorithm>
#include <catch2/catch.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <variant>
#include <vector>
using namespace fol::lexer::literals;
using namespace fol::parser;

void TestParsing(std::string str) {
  REQUIRE(ToString(Parse(fol::lexer::Tokenize(str))) == str);
}

TEST_CASE("simple std::cout output", "[parser][fol]") {
  TestParsing("@ va . (pP1(c1) and pP2(c3))");
  TestParsing("@ vx . pP1(vx)->pP2(vx)->pP3(vx)->pP4(vx)");
  TestParsing("@ vx . pHuman(vx)->pMortal(vx)");
  TestParsing(
      "((@ vx . (pHuman(vx)->pMortal(vx)) and "
      "pHuman(cSocrates))->pMortal(cSocrates))");
  TestParsing("@ vx . pHuman(vx)->pHuman(fParent(vx))");
}

TEST_CASE("test operator overloading", "[parser][fol]") {
  REQUIRE(ToString("cC1"_t |= "cC2"_t) == "cC1, cC2");
  REQUIRE(ToString("vx"_t) == "vx");
  REQUIRE(ToString("Func"_f * ("cC1"_t |= "cC2"_t)) == "fFunc(cC1, cC2)");
  REQUIRE(ToString("Func"_f * ("cC1"_t |= "vx"_t |= "cC2"_t)) ==
          "fFunc(cC1, vx, cC2)");
  REQUIRE(ToString("Func"_f * ("cC1"_t |= "vx"_t |= "cC2"_t) |= "vx"_t |=
                   "cC3"_t) == "fFunc(cC1, vx, cC2), vx, cC3");
  REQUIRE(ToString(ForAll("x"_v,
                          ("Human"_p * ("vx"_t)) >>= ("Alive"_p * ("vx"_t)))) ==
          "@ vx . pHuman(vx)->pAlive(vx)");
  REQUIRE(ToString(Exists("x"_v,
                          ("Human"_p * ("vx"_t)) >>= ("Alive"_p * ("vx"_t)))) ==
          "? vx . pHuman(vx)->pAlive(vx)");
}

