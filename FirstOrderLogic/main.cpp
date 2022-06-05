#include <cstdlib>
#include <iostream>
#include <libfol-basictypes/basic_clauses_storage.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-prover/prover.hpp>
#include <libfol-transform/normalization.hpp>
#include <libfol-transform/normalized_formula.hpp>
#include <libfol-unification/martelli_montanari_unification.hpp>
#include <libfol-unification/robinson_unification.hpp>
#include <numeric>
#include <optional>

std::optional<fol::parser::FolFormula> Parse(std::string str) {
  try {
    auto ret = fol::parser::Parse(fol::lexer::Tokenize(str));

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

template <class T>
T inputToEnd(std::istream& is) {
  T res{};
  T temp;
  while (std::getline(is, temp)) {
    res += temp;
  }
  return res;
}

fol::parser::FolFormula ReadLastFormula() {
  std::optional<fol::parser::FolFormula> formula;
  while (!(formula = Parse(inputToEnd<std::string>(std::cin)))) {
  }
  return std::move(*formula);
}

fol::parser::FolFormula ReadFormula() {
  std::optional<fol::parser::FolFormula> formula;
  while (!(formula = Parse(input<std::string>(std::cin)))) {
  }
  return std::move(*formula);
}

std::vector<fol::types::Clause> ClausesFromFol(
    fol::parser::FolFormula formula) {
  std::vector<fol::types::Clause> res;
  auto norm_formula = fol::transform::ToNormalizedFormula(
      fol::transform::Normalize(std::move(formula)));

  std::cout << norm_formula << std::endl;
  norm_formula.Skolemize();
  auto disjs = norm_formula.GetDisjunctions();
  res.reserve(disjs.size());

  for (auto& disj : disjs) {
    res.push_back(std::move(disj));
  }

  return res;
}

int main() {
  std::cout << "Enter axioms' number: ";
  int axioms_count;
  std::cin >> axioms_count;
  std::vector<fol::parser::FolFormula> axioms;
  axioms.reserve(axioms_count);

  for (int i = 0; i < axioms_count; ++i) {
    axioms.push_back(ReadFormula());
  }

  std::vector<fol::types::Clause> clauses;

  for (auto& a : axioms) {
    std::cout << "Axiom: " << a << std::endl;
    auto a_cls = ClausesFromFol(std::move(a));
    clauses.insert(clauses.cend(), a_cls.begin(), a_cls.end());
  }

  std::cout << "Enter hypothesis: ";
  fol::parser::FolFormula hypothesis = ToFol(~!ReadLastFormula());
  auto a_cls = ClausesFromFol(std::move(hypothesis));
  clauses.insert(clauses.cend(), a_cls.begin(), a_cls.end());

  for (auto& c : clauses) {
    std::cout << c << std::endl;
  }

  auto prover = fol::prover::Prover(
      std::make_unique<fol::unification::MartelliMontanariUnificator>(),
      std::make_unique<fol::types::BasicClausesStorage>(std::move(clauses)),
      std::make_unique<fol::types::BasicClausesStorage>());

  std::cout << std::boolalpha << prover.Provable() << std::endl;
}
