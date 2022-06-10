#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/normalized_formula.hpp>
#include <libfol-transform/replace.hpp>
#include <numeric>

namespace fol::transform {
class NormalizedFormula;
NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula);

std::ostream &operator<<(std::ostream &os, NormalizedFormula const &formula) {
  for (auto &quantifier : formula.quantifiers_) {
    os << (quantifier.type == NormalizedFormula::Quantifier::Type::Exists ? '?'
                                                                          : '@')
       << " " << quantifier.variable << " . ";
  }

  os << fol::parser::ToString(formula.formula_matrix_);
  return os;
}

void NormalizedFormula::Skolemize() {
  std::vector<Quantifier> new_quantifiers;
  new_quantifiers.reserve(quantifiers_.size());

  for (std::vector<Quantifier>::size_type i = 0; i < quantifiers_.size(); ++i) {
    if (quantifiers_[i].type == Quantifier::Type::Forall) {
      new_quantifiers.push_back(quantifiers_[i]);
      continue;
    }

    if (new_quantifiers.empty()) {
      auto fun = UniqFunName();
      std::string new_n_fun = fun + "(cEMPTY)";
      auto generator = lexer::Tokenize(new_n_fun);
      auto it = generator.begin();
      auto new_n_fun_t = parser::ParseTerm(it);
      transform::ReplaceTermVar(formula_matrix_, quantifiers_[i].variable,
                                new_n_fun_t);
    } else {
      std::string new_n_fun =
          std::accumulate(new_quantifiers.begin() + 1, new_quantifiers.end(),
                          UniqFunName() + "(" + new_quantifiers[0].variable,
                          [](auto &&lhs, auto &&rhs) {
                            return lhs + ", " + rhs.variable;
                          }) +
          ")";
      auto generator = lexer::Tokenize(new_n_fun);
      auto it = generator.begin();
      auto new_n_fun_t = parser::ParseTerm(it);
      transform::ReplaceTermVar(formula_matrix_, quantifiers_[i].variable,
                                new_n_fun_t);
    }
  }

  quantifiers_ = std::move(new_quantifiers);
}

std::vector<parser::FolFormula> NormalizedFormula::GetDisjunctions() const {
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

fol::parser::FolFormula ExtractAllQuantifiers(
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

NormalizedFormula ToNormalizedFormula(fol::parser::FolFormula formula) {
  std::vector<NormalizedFormula::Quantifier> quantifiers;
  auto formula_matrix = ExtractAllQuantifiers(std::move(formula), quantifiers);
  quantifiers.shrink_to_fit();
  return NormalizedFormula{std::move(quantifiers), std::move(formula_matrix)};
}

}  // namespace fol::transform

