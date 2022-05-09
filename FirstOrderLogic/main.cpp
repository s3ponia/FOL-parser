#include <cstdlib>
#include <iostream>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-transform/normalization.hpp>
#include <libfol-transform/normalized_formula.hpp>
#include <libfol-unification/robinson_unification.hpp>
#include <optional>

std::optional<fol::transform::NormalizedFormula> Parse(std::string str) {
  try {
    auto ret = fol::transform::ToNormalizedFormula(fol::transform::Normalize(
        fol::parser::Parse(fol::lexer::Tokenize(str))));

    ret.Skolemize();
    return ret;
  } catch (const fol::parser::ParseError& e) {
    std::cerr << "Error in parsing at position " << fol::lexer::i << " \'"
              << str[fol::lexer::i] << "\': " << e.what() << std::endl;
    return std::nullopt;
  } catch (const fol::lexer::LexerError& e) {
    std::cerr << "Error in lexing at position " << fol::lexer::i << " \'"
              << str[fol::lexer::i] << "\': " << e.what() << std::endl;
    return std::nullopt;
  }
}

template <class T>
T input(std::istream& is) {
  T res;
  std::getline(is, res);
  return res;
}

fol::transform::NormalizedFormula ReadFormula() {
  std::optional<fol::transform::NormalizedFormula> formula;
  while (!(formula = Parse(input<std::string>(std::cin)))) {
  }
  return std::move(*formula);
}

int main() {
  std::cout << "Enter axioms' number: ";
  int axioms_count;
  std::cin >> axioms_count;
  std::vector<fol::transform::NormalizedFormula> axioms;
  axioms.reserve(axioms_count);

  for (int i = 0; i < axioms_count; ++i) {
    axioms.push_back(ReadFormula());
  }

  std::cout << "Enter hypothesis: ";
  fol::transform::NormalizedFormula hypothesis = ReadFormula();

  std::cout << "Choose unification algorithm:\n"
               "1. Robinson unification algorithm\n"
               "2. Prokhorov unification algorithm\n"
               "3. Martelli-Montanari unification algorithm\n";
  int unification_algo;
  std::cin >> unification_algo;

  std::cout << "Choose clauses choosing algorithm:\n"
               "1. Метод насыщения уровня\n"
               "2. Стратегия предпочтения более коротких дизъюнктов\n"
               "3. Стратегия вычеркивания\n"
               "4. Линейная резолюция\n"
               "5. Стратегия поддержки\n"
               "6. Limited Resources Strategy\n";
  int clauses_choosing_algorithm;
  std::cin >> clauses_choosing_algorithm;
}
