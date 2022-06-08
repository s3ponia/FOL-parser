#include <libfol-basictypes/term.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/replace.hpp>

namespace fol::types {
Term Clone(const Term& t) { return transform::CloneTerm(t); }
bool operator==(const Term& lhs, const Term& rhs) {
  return parser::ToString(lhs) == parser::ToString(rhs);
}
bool Contains(const Term& term, const Term& var) {
  return parser::ToString(term).find(parser::ToString(var)) !=
         std::string::npos;
}
}  // namespace fol::types
