#include "mock_interrupt_source.hpp"

void trigger_conflicting_handlers() noexcept {
  grevir_poc::MockInterruptSource<grevir_poc::ConflictingTick>::fire();
}
