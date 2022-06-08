#pragma once

#include <libfol-parser/parser/types.hpp>

namespace fol::types {
using parser::Term;
Term Clone(const Term& t);
bool operator==(const Term& lhs, const Term& rhs);
bool Contains(const Term& term, const Term& var);
}  // namespace fol::types
