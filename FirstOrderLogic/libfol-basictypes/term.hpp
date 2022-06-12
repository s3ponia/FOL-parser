#pragma once

#include <libfol-parser/parser/types.hpp>

namespace fol::types {
using parser::Term;
Term Clone(const Term& t);
bool Contains(const Term& term, const Term& var);
}  // namespace fol::types
