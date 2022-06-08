#include <details/utils/utility.hpp>
#include <libfol-basictypes/clause.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/normalization.hpp>

namespace fol::types {
namespace {
static std::vector<Atom> DisjunctionFormulaToAtoms(parser::FolFormula disj) {
  disj = transform::DeleteUselessBrackets(std::move(disj));
  auto str = parser::ToString(disj);

  auto atoms_str = details::utils::Split(str, " or ");

  std::vector<Atom> atoms;
  atoms.reserve(atoms_str.size());

  for (auto& s : atoms_str) {
    atoms.push_back(Atom(parser::Parse(lexer::Tokenize(s))));
  }

  std::sort(atoms.begin(), atoms.end());

  return atoms;
}
}  // namespace

Clause::Clause(parser::FolFormula disj)
    : atoms_(DisjunctionFormulaToAtoms(std::move(disj))) {}
std::ostream& operator<<(std::ostream& os, const Clause& c) {
  if (c.atoms_.empty()) {
    os << "EMPTY";
    return os;
  }
  for (std::size_t i = 0; i < c.atoms_.size(); ++i) {
    os << c.atoms_[i] << (i == c.atoms_.size() - 1 ? "" : " or ");
  }
  return os;
}

Clause& Clause::operator+=(const Clause& o) {
  atoms_.reserve(atoms_.size() + o.atoms_.size());
  atoms_.insert(atoms_.cend(), o.atoms_.begin(), o.atoms_.end());
  std::sort(atoms_.begin(), atoms_.end());
  return *this;
}
}  // namespace fol::types
