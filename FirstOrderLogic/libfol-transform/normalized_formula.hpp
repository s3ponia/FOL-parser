#pragma once

#include <libfol-parser/parser/types.hpp>
#include <string>
#include <vector>

namespace fol::transform {
class NormalizedFormula;
NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula);

class NormalizedFormula {
 public:
  friend NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula);
  friend std::ostream &operator<<(std::ostream &os,
                                  NormalizedFormula const &formula);

  std::vector<parser::FolFormula> GetDisjunctions() const;

  parser::FolFormula ToFol() const;

  struct Quantifier {
    enum class Type { Forall, Exists };
    Type type;
    std::string variable;
  };

 private:
  NormalizedFormula(std::vector<Quantifier> quantifiers,
                    fol::parser::FolFormula formula_matrix)
      : quantifiers_(std::move(quantifiers)),
        formula_matrix_(std::move(formula_matrix)) {}

  std::vector<Quantifier> quantifiers_;
  fol::parser::FolFormula formula_matrix_;
};

}  // namespace fol::transform

