#include "mock_event_app.hpp"
#include <grevir/interrupt/start.hpp>

int main() {
  const auto started = grevir::interrupt::Application<GrevirApplication>::start();
  if (started.outcome != grevir::interrupt::SetupOutcome::success) { return 1; }
  mock_app::controller.raise();
  return mock_app::ticks == 1 ? 0 : 2;
}
