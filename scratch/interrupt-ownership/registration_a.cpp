#include "registration.hpp"

#if defined(GREVIR_IRQ_STRONG_GUARD)
extern "C" {
int grevir_irq_owner_uart0 = 0;
}
#endif

namespace probe {

void register_first() { mock_esp_intr_alloc(0); }

} // namespace probe
