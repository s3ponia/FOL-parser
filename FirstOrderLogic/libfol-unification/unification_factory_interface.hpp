#pragma once

#include <libfol-unification/unification_interface.hpp>

namespace fol::unification {
class IUnificatorFactory {
 public:
  virtual std::unique_ptr<IUnificator> create() = 0;
};
}  // namespace fol::unification
