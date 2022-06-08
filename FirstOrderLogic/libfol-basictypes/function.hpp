#pragma once

#include <libfol-basictypes/term.hpp>
#include <libfol-basictypes/variable.hpp>
#include <libfol-parser/parser/types.hpp>
#include <string>
#include <vector>

namespace fol::types {
class Function {
 public:
  friend std::ostream& operator<<(std::ostream& os, const Function& function);

  Function(parser::FunctionFormula formula);

  Function(const Function& a);

  Function& operator=(const Function& a);

  void swap(Function& o);

  bool operator==(const Function& o) const;

  bool operator<(const Function& o) const;

  void Substitute(const Variable& from, const Term& to);

  const Term& operator[](std::size_t i) const { return term_list_[i]; }
  const auto& terms() const { return term_list_; }
  auto& terms() { return term_list_; }
  std::size_t terms_size() const { return term_list_.size(); }
  const std::string& function_name() const { return function_name_; }

 private:
  std::string function_name_;
  std::vector<Term> term_list_;
};
}  // namespace fol::types
