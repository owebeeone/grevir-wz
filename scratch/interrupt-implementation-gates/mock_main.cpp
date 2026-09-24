#include "mock_app.hpp"
#include <grevir/interrupt/start.hpp>

int main() {
  const auto started = grevir::interrupt::Application<GrevirApplication>::start();
  if (started.outcome != grevir::interrupt::SetupOutcome::success
      || started.disposition != grevir::interrupt::CallDisposition::initiated) { return 1; }
  mock_app::controller.raise();
  const auto repeated = grevir::interrupt::Application<GrevirApplication>::start();
  return mock_app::ticks == 1
    && repeated.outcome == grevir::interrupt::SetupOutcome::success
    && repeated.disposition == grevir::interrupt::CallDisposition::replayed ? 0 : 2;
}
