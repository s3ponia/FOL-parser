#include <libfol-unification/here_unification.hpp>
#include <libfol-unification/here_unification_factory.hpp>

namespace fol::unification {
std::unique_ptr<IUnificator> HereUnificatorFactory::create() {
  return std::make_unique<HereUnificator>();
}
}  // namespace fol::unification
