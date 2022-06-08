#pragma once

#include <libfol-unification/unification_factory_interface.hpp>

namespace fol::unification {
class RobinsonUnificatorFactory : public IUnificatorFactory {
 public:
  std::unique_ptr<IUnificator> create() override;
};
}  // namespace fol::unification

