#include <cstdlib>
#include <iostream>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-transform/normalization.hpp>
#include <libfol-transform/normalized_formula.hpp>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::printf("usage: %s [FOL formula]", argv[0]);
    return EXIT_FAILURE;
  }

  auto formula = fol::transform::ToNormalizedFormula(fol::transform::Normalize(
      fol::parser::Parse(fol::lexer::Tokenize(argv[1]))));
  formula.Skolemize();

  std::cout << formula << std::endl;
}
