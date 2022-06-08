#pragma once

#include <libfol-matcher/check_matcher.hpp>
#include <libfol-matcher/matcher.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <libfol-transform/replace.hpp>

#include "details/utils/utility.hpp"

namespace fol::transform {
parser::FolFormula Normalize(parser::FolFormula formula);

parser::FolFormula DeleteUselessBrackets(parser::FolFormula formula);
}  // namespace fol::transform

