#include <libfol-basictypes/term.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-transform/replace.hpp>
#include <type_traits>

namespace fol::types {
Term Clone(const Term& t) { return transform::CloneTerm(t); }
bool Contains(const parser::FunctionFormula& function, const Term& var) {
  for (auto it = parser::FunctionTermsIt(function);
       it != parser::ConstTermListIt{}; ++it) {
    if (*it == var) {
      return true;
    } else if (it->IsFunction()) {
      return Contains(it->Function(), var);
    }
  }

  return false;
}
bool Contains(const Term& term, const Term& var) {
  if (!term.IsFunction()) {
    return term == var;
  }
  return Contains(term.Function(), var);
}
}  // namespace fol::types
