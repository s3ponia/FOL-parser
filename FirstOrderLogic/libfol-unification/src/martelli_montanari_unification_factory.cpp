#include <libfol-unification/martelli_montanari_unification.hpp>
#include <libfol-unification/martelli_montanari_unification_factory.hpp>

namespace fol::unification {
std::unique_ptr<IUnificator> MartelliMontanariUnificatorFactory::create() {
  return std::make_unique<MartelliMontanariUnificator>();
}
}  // namespace fol::unification
