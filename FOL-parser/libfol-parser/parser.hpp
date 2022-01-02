#pragma once

#include <details/utils/utility.hpp>
#include <string>
#include <variant>

namespace fol_parser {
struct OpenBracket {};
struct CloseBracket {};
struct Forall {};
struct Exists {};
struct And {};
struct Or {};
struct Implies {};
struct Not {};
struct Coma {};
using Function =
    details::utils::TypeWithLabel<std::string, class FunctionLabel>;
using Variable =
    details::utils::TypeWithLabel<std::string, class VariableLabel>;
using Predicate =
    details::utils::TypeWithLabel<std::string, class PredicateLabel>;
using Constant =
    details::utils::TypeWithLabel<std::string, class ConstantLabel>;

using Lexem =
    std::variant<OpenBracket, CloseBracket, Forall, Exists, And, Or, Implies,
                 Not, Coma, Function, Variable, Predicate, Constant>;
}  // namespace fol_parser

