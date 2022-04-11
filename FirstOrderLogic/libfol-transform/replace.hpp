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

template <class T>
inline T RenameVar(T src, std::string with) {
  auto str = parser::ToString(src.data.second);
  ReplaceAll(str, src.data.first, with);
  src.data.second = parser::Parse(lexer::Tokenize(str));
  src.data.first = with;
  return src;
}

template <class T>
inline T RenameVar(T src) {
  static int cnt = 0;
  auto with = "vu" + std::to_string(cnt++);
  return RenameVar(std::move(src), with);
}

inline parser::FolFormula RenameVar(parser::FolFormula src,
                                    lexer::Variable what,
                                    lexer::Variable with) {
  auto str = parser::ToString(src);
  ReplaceAll(str, what, with);
  return parser::Parse(lexer::Tokenize(str));
}
}  // namespace fol::transform

