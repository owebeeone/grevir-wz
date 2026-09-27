#include "mock_interrupt_source.hpp"

void trigger_missing_handler() noexcept {
  grevir_poc::MockInterruptSource<grevir_poc::MissingTick>::fire();
}
