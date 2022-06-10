#pragma once

#include <cstdint>
#include <libfol-basictypes/term.hpp>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/parser.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-parser/parser/types.hpp>
#include <string>
#include <string_view>
#include <utility>

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

inline int cnt = 0;
template <class T>
inline T RenameVar(T src) {
  ++cnt;
  auto with = "vu" + std::to_string(cnt);
  return RenameVar(std::move(src), with);
}

template <class T>
inline parser::Term CloneTerm(T&& src) {
  auto str = parser::ToString(std::forward<T>(src));
  auto generator = lexer::Tokenize(str);
  auto it = generator.begin();
  return parser::ParseTerm(it);
}

template <class T>
inline parser::FolFormula ReplaceWithConst(T&& src, std::string what) {
  auto str = parser::ToString(std::forward<T>(src));
  ++cnt;
  auto with = "cu" + std::to_string(cnt);
  ReplaceAll(str, what, with);
  return parser::Parse(lexer::Tokenize(str));
}

template <class T>
inline parser::FolFormula Replace(T&& src, std::string what) {
  auto str = parser::ToString(std::forward<T>(src));
  ++cnt;
  auto with = "vu" + std::to_string(cnt);
  ReplaceAll(str, what, with);
  return parser::Parse(lexer::Tokenize(str));
}

template <class T>
inline parser::FolFormula Replace(T&& src, std::string what, std::string with) {
  auto str = parser::ToString(std::forward<T>(src));
  ReplaceAll(str, what, with);
  return parser::Parse(lexer::Tokenize(str));
}

inline void ReplaceTerm(parser::ImplicationFormula& where,
                        const parser::Term& from, const parser::Term& to);
inline void ReplaceTerm(parser::UnaryFormula& where, const parser::Term& from,
                        const parser::Term& to);
inline void ReplaceTerm(parser::Term& where, const parser::Term& from,
                        const parser::Term& to);

inline void ReplaceTerm(parser::PredicateFormula& where,
                        const parser::Term& from, const parser::Term& to) {
  for (auto it = parser::TermListIt{&where.data.second};
       it != parser::TermListIt{}; ++it) {
    ReplaceTerm(*it, from, to);
  }
}

inline void ReplaceTerm(parser::ExistsFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  ReplaceTerm(where.data.second, from, to);
}

inline void ReplaceTerm(parser::ForallFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  ReplaceTerm(where.data.second, from, to);
}

inline void ReplaceTerm(parser::NotFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  ReplaceTerm(*where.data, from, to);
}

inline void ReplaceTerm(parser::BracketFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  ReplaceTerm(where.data, from, to);
}

inline void ReplaceTerm(parser::UnaryFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  std::visit([&](auto&& a) { ReplaceTerm(a, from, to); }, where.data);
}

inline void ReplaceTerm(parser::ConjunctionPrimeFormula& where,
                        const parser::Term& from, const parser::Term& to) {
  std::visit(
      [&](auto&& a) {
        if constexpr (details::utils::Dereferencable<decltype(a)>) {
          ReplaceTerm(a->first, from, to);
          ReplaceTerm(a->second, from, to);
        }
      },
      where.data);
}

inline void ReplaceTerm(parser::ConjunctionFormula& where,
                        const parser::Term& from, const parser::Term& to) {
  ReplaceTerm(where.data->first, from, to);
  ReplaceTerm(where.data->second, from, to);
}

inline void ReplaceTerm(parser::DisjunctionPrimeFormula& where,
                        const parser::Term& from, const parser::Term& to) {
  std::visit(
      [&](auto&& a) {
        if constexpr (details::utils::Dereferencable<decltype(a)>) {
          ReplaceTerm(a->first, from, to);
          ReplaceTerm(a->second, from, to);
        }
      },
      where.data);
}

inline void ReplaceTerm(parser::DisjunctionFormula& where,
                        const parser::Term& from, const parser::Term& to) {
  ReplaceTerm(where.data.first, from, to);
  ReplaceTerm(where.data.second, from, to);
}

inline void ReplaceTerm(parser::FolFormula& where, const parser::Term& from,
                        const parser::Term& to) {
  std::visit(
      [&](auto&& a) {
        if constexpr (details::utils::Dereferencable<decltype(a)>) {
          ReplaceTerm(a->first, from, to);
          ReplaceTerm(a->second, from, to);
        } else {
          ReplaceTerm(a, from, to);
        }
      },
      where.data);
}

inline void ReplaceTerm(parser::Term& where, const parser::Term& from,
                        const parser::Term& to) {
  if (where == from) {
    where = types::Clone(to);
    return;
  }

  if (where.IsFunction()) {
    for (auto it = parser::FunctionTermsIt(where.Function());
         it != parser::TermListIt{}; ++it) {
      ReplaceTerm(*it, from, to);
    }
  }
}

inline void ReplaceTermVar(auto& where, const std::string& from,
                           const parser::Term& to) {
  lexer::Variable var;
  var.base() = from;
  ReplaceTerm(where, parser::Term{var}, to);
}

inline parser::FolFormula RenameVar(parser::FolFormula src, std::string what,
                                    std::string with) {
  return Replace(src, what, with);
}
}  // namespace fol::transform

