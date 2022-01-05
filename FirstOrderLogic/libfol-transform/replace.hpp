#pragma once

#include <cstdint>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <string>
#include <string_view>

namespace fol::transform {
inline std::size_t ReplaceAll(std::string& inout, std::string_view what,
                              std::string_view with) {
  std::size_t count{};
  for (std::string::size_type pos{};
       inout.npos != (pos = inout.find(what.data(), pos, what.length()));
       pos += with.length(), ++count) {
    inout.replace(pos, what.length(), with.data(), with.length());
  }
  return count;
}

inline parser::FolFormula RenameVar(parser::FolFormula src,
                                    lexer::Variable what,
                                    lexer::Variable with) {
  auto str = parser::ToString(src);
  ReplaceAll(str, what, with);
  return parser::Parse(lexer::Tokenize(str));
}
}  // namespace fol::transform

