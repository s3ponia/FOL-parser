#pragma once

#include <stdexcept>

namespace fol::parser {
struct ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
}  // namespace fol::parser

