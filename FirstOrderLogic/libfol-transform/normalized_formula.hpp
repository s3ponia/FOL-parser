#pragma once

#include <iostream>
#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-transform/replace.hpp>
#include <numeric>
#include <vector>

#include "libfol-parser/lexer/lexer.hpp"
#include "libfol-parser/parser/parser.hpp"

namespace fol::transform {
class NormalizedFormula;
NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula);

class NormalizedFormula {
 public:
  friend NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula);
  friend std::ostream &operator<<(std::ostream &os,
                                  NormalizedFormula const &formula) {
    for (auto &quantifier : formula.quantifiers_) {
      os << (quantifier.type == Quantifier::Type::Exists ? '?' : '@') << " "
         << quantifier.variable << " . ";
    }

    os << fol::parser::ToString(formula.formula_matrix_);
    return os;
  }

  void Skolemize() {
    std::vector<Quantifier> new_quantifiers;
    new_quantifiers.reserve(quantifiers_.size());

    for (std::vector<Quantifier>::size_type i = 0; i < quantifiers_.size();
         ++i) {
      if (quantifiers_[i].type == Quantifier::Type::Forall) {
        new_quantifiers.push_back(quantifiers_[i]);
        continue;
      }

      if (new_quantifiers.empty()) {
        formula_matrix_ =
            Replace(std::move(formula_matrix_), quantifiers_[i].variable + ",",
                    UniqFunName() + "(cEMPTY),");
        formula_matrix_ =
            Replace(std::move(formula_matrix_), quantifiers_[i].variable + ")",
                    UniqFunName() + "(cEMPTY))");
      } else {
        std::string new_n_fun =
            std::accumulate(new_quantifiers.begin() + 1, new_quantifiers.end(),
                            UniqFunName() + "(" + new_quantifiers[0].variable,
                            [](auto &&lhs, auto &&rhs) {
                              return lhs + ", " + rhs.variable;
                            }) +
            ")";
        formula_matrix_ =
            Replace(std::move(formula_matrix_), quantifiers_[i].variable + ",",
                    new_n_fun + ",");
        formula_matrix_ =
            Replace(std::move(formula_matrix_), quantifiers_[i].variable + ")",
                    new_n_fun + ")");
      }
    }

    quantifiers_ = std::move(new_quantifiers);
  }

  std::vector<parser::FolFormula> GetDisjunctions() const {
    std::vector<parser::FolFormula> res;

    auto matrix_s = ToString(formula_matrix_);
    auto disjunctions_s = details::utils::Split(matrix_s, " and ");

    res.reserve(disjunctions_s.size());

    for (auto &s : disjunctions_s) {
      res.push_back(parser::Parse(lexer::Tokenize(s)));
    }

    return res;
  }

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

  std::string UniqFunName() {
    static int cnt = 0;
    return "funiq" + std::to_string(cnt++);
  }

  std::vector<Quantifier> quantifiers_;
  fol::parser::FolFormula formula_matrix_;
};

inline fol::parser::FolFormula ExtractAllQuantifiers(
    fol::parser::FolFormula formula,
    std::vector<NormalizedFormula::Quantifier> &output_quantifiers) {
  if (fol::matcher::check::Exists()(formula)) {
    std::optional<fol::parser::ExistsFormula> exists_f;
    fol::matcher::RefExists(exists_f).match(std::move(formula));
    output_quantifiers.emplace_back(NormalizedFormula::Quantifier::Type::Exists,
                                    exists_f.value().data.first);
    return ExtractAllQuantifiers(std::move(exists_f.value().data.second),
                                 output_quantifiers);
  }

  if (fol::matcher::check::Forall()(formula)) {
    std::optional<fol::parser::ForallFormula> forall_f;
    fol::matcher::RefForall(forall_f).match(std::move(formula));
    output_quantifiers.emplace_back(NormalizedFormula::Quantifier::Type::Forall,
                                    forall_f.value().data.first);
    return ExtractAllQuantifiers(std::move(forall_f.value().data.second),
                                 output_quantifiers);
  }

  return formula;
}

inline NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula formula) {
  std::vector<NormalizedFormula::Quantifier> quantifiers;
  auto formula_matrix = ExtractAllQuantifiers(std::move(formula), quantifiers);
  quantifiers.shrink_to_fit();
  return NormalizedFormula{std::move(quantifiers), std::move(formula_matrix)};
}

}  // namespace fol::transform

