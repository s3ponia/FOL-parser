#include <chrono>
#include <cstdlib>
#include <iostream>
#include <libfol-basictypes/basic_clauses_storage.hpp>
#include <libfol-basictypes/basic_clauses_storage_factory.hpp>
#include <libfol-basictypes/short_precedence_clauses_storage.hpp>
#include <libfol-basictypes/short_precedence_clauses_storage_factory.hpp>
#include <libfol-basictypes/strikeout_clauses_storage.hpp>
#include <libfol-basictypes/strikeout_clauses_storage_factory.hpp>
#include <libfol-basictypes/support_clauses_storage_factory.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-prover/prover.hpp>
#include <libfol-transform/normalization.hpp>
#include <libfol-transform/normalized_formula.hpp>
#include <libfol-unification/here_unification_factory.hpp>
#include <libfol-unification/martelli_montanari_unification_factory.hpp>
#include <libfol-unification/robinson_unification_factory.hpp>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>

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
  is >> res;
  return res;
}

template <class T>
T input_line(std::istream& is) {
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
  while (!(formula = Parse(input_line<std::string>(std::cin)))) {
  }
  return std::move(*formula);
}

std::vector<fol::types::Clause> ClausesFromFol(
    fol::parser::FolFormula formula) {
  std::vector<fol::types::Clause> res;
  auto norm_formula = fol::transform::ToNormalizedFormula(
      fol::transform::Normalize(std::move(formula)));

  norm_formula.Skolemize();
  std::cout << "Normalized and skolemized formula: " << norm_formula
            << std::endl;
  auto disjs = norm_formula.GetDisjunctions();
  res.reserve(disjs.size());

  for (auto& disj : disjs) {
    res.push_back(std::move(disj));
  }

  return res;
}

void CollectAncestors(const fol::types::Clause& clause,
                      std::map<std::size_t, fol::types::Clause>& map) {
  auto& ancestors = clause.ancestors();
  for (auto& clause : ancestors) {
    CollectAncestors(clause, map);
  }
  map[clause.id()] = clause;
}

void PrintProof(const fol::types::Clause& clause) {
  std::map<std::size_t, fol::types::Clause> map;
  CollectAncestors(clause, map);

  for (auto& [k, v] : map) {
    std::cout << "[" << k << "] " << v;
    auto& ancestors = v.ancestors();
    std::cout << "[ ";
    if (ancestors.empty()) {
      std::cout << "AXIOM ";
    }
    for (auto& clause : ancestors) {
      std::cout << clause.id() << " ";
    }
    std::cout << "]\n";
  }

  std::cout << "Proof size: " << map.size() << std::endl;
  std::cout << "Useless clauses: " << clause.id() - map.size() << std::endl;
}

int main() {
  std::cout << "Choose unification algorithm:\n"
               "[1] Robinson unification\n"
               "[2] Here unification\n"
               "[3] Martelli-Montanari unification\n";
  std::shared_ptr<fol::unification::IUnificatorFactory> unification_factories[]{
      std::make_shared<fol::unification::RobinsonUnificatorFactory>(),
      std::make_shared<fol::unification::HereUnificatorFactory>(),
      std::make_shared<fol::unification::MartelliMontanariUnificatorFactory>()};

  auto unification_factory =
      std::move(unification_factories[input<int>(std::cin) - 1]);

  std::cout << "Choose clause choosing policy:\n"
               "[1] Saturation policy\n"
               "[2] Short precedence policy\n"
               "[3] Strikeout policy\n"
               "[4] Support policy\n"
               "[5] Strikeout + Short precedence policy\n"
               "[6] Support + Short precedence policy\n";
  std::shared_ptr<fol::types::IClausesStorageFactory>
      clauses_storage_factories[]{
          std::make_shared<fol::types::BasicClausesStorageFactory>(),
          std::make_shared<fol::types::ShortPrecedenceClausesStorageFactory>(),
          std::make_shared<fol::types::StrikeoutClausesStorageFactory<
              fol::types::BasicClausesStorage>>(unification_factory),
          std::make_shared<fol::types::SupportClausesStorageFactory<
              fol::types::BasicClausesStorage>>(),
          std::make_shared<fol::types::StrikeoutClausesStorageFactory<
              fol::types::ShortPrecedenceClausesStorage>>(unification_factory),
          std::make_shared<fol::types::SupportClausesStorageFactory<
              fol::types::ShortPrecedenceClausesStorage>>()};
  auto clauses_storage_factory =
      std::move(clauses_storage_factories[input<int>(std::cin) - 1]);

  std::cout << "Enter axioms' number: ";
  int axioms_count;
  std::cin >> axioms_count;
  std::vector<fol::parser::FolFormula> axioms;
  axioms.reserve(axioms_count);

  for (int i = 0; i < axioms_count; ++i) {
    axioms.push_back(ReadFormula());
  }

  std::vector<fol::types::Clause> axiom_clauses;

  for (auto& a : axioms) {
    std::cout << "Axiom: " << a << std::endl;
    auto a_cls = ClausesFromFol(std::move(a));
    axiom_clauses.insert(axiom_clauses.cend(), a_cls.begin(), a_cls.end());
  }

  std::cout << "Enter hypothesis: ";
  fol::parser::FolFormula hypothesis = ToFol(~!ReadLastFormula());
  auto hypothesis_clauses = ClausesFromFol(std::move(hypothesis));

  auto tm_un = unification_factory->create();

  for (auto& c : axiom_clauses) {
    tm_un->Simplify(c);
    std::cout << "[" << c.id() << "] " << c << std::endl;
  }

  for (auto& c : hypothesis_clauses) {
    tm_un->Simplify(c);
    std::cout << "[" << c.id() << "] " << c << std::endl;
  }

  auto clauses_storages = clauses_storage_factory->create(
      std::move(axiom_clauses), std::move(hypothesis_clauses));

  auto prover = fol::prover::Prover(unification_factory->create(),
                                    std::move(clauses_storages.first),
                                    std::move(clauses_storages.second));

  auto start = std::chrono::steady_clock::now();
  auto res = prover.Prove();
  auto end = std::chrono::steady_clock::now();

  if (res) {
    PrintProof(*res);
  } else {
    std::cout << "No proof" << std::endl;
  }

  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Elapsed time: " << 1000 * elapsed_seconds.count() << "ms\n";
}
