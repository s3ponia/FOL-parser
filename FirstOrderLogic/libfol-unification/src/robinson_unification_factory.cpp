#include <libfol-unification/robinson_unification.hpp>
#include <libfol-unification/robinson_unification_factory.hpp>

namespace fol::unification {
std::unique_ptr<IUnificator> RobinsonUnificatorFactory::create() {
  return std::make_unique<RobinsonUnificator>();
}
}  // namespace fol::unification
