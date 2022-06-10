#include <libfol-basictypes/function.hpp>
#include <libfol-parser/parser/print.hpp>
#include <libfol-transform/replace.hpp>

namespace fol::types {
Function::Function(parser::FunctionFormula formula)
    : function_name_(formula.data->first),
      term_list_(FromTermList(std::move(formula.data->second))) {}

Function::Function(const Function& a) : function_name_(a.function_name_) {
  term_list_.reserve(a.terms_size());
  for (auto& t : a.term_list_) {
    term_list_.push_back(Clone(t));
  }
}

Function& Function::operator=(const Function& a) {
  auto copy = a;
  copy.swap(*this);
  return *this;
}

void Function::swap(Function& o) {
  o.term_list_.swap(term_list_);
  o.function_name_.swap(function_name_);
}

bool Function::operator==(const Function& o) const {
  return function_name_ == o.function_name_ &&
         term_list_.size() == o.term_list_.size() &&
         std::equal(term_list_.begin(), term_list_.end(), o.term_list_.begin());
}

bool Function::operator<(const Function& o) const {
  return parser::ToString(*this) < parser::ToString(o);
}

void Function::Substitute(const Variable& from, const Term& to) {
  for (auto& term : term_list_) {
    transform::ReplaceTermVar(term, from, to);
  }
}

std::ostream& operator<<(std::ostream& os, const Function& function) {
  os << function.function_name_ << "(";
  if (!function.term_list_.empty()) {
    os << parser::ToString(function.term_list_[0]);
  }
  for (std::vector<Term>::size_type i = 1; i < function.term_list_.size();
       ++i) {
    os << ", " << parser::ToString(function.term_list_[i]);
  }
  os << ")";
  return os;
}
}  // namespace fol::types
