#include <libfol-basictypes/term.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/replace.hpp>
#include <type_traits>

namespace fol::types {
Term Clone(const Term& t) { return transform::CloneTerm(t); }
bool Contains(const Term& term, const Term& var) {
  return parser::ToString(term).find(parser::ToString(var)) !=
         std::string::npos;
}
}  // namespace fol::types
