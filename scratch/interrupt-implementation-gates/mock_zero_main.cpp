#include "mock_app_base.hpp"
#include <grevir/interrupt/start.hpp>

int main() {
  const auto result = grevir::interrupt::Application<GrevirApplication>::start();
  if (result.outcome != grevir::interrupt::SetupOutcome::success) { return 1; }
  return mock_app::controller.installed() || mock_app::controller.enabled() ? 2 : 0;
}
