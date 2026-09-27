#include "mock_app.hpp"
#include <grevir/interrupt/start.hpp>
#include <cstdio>

int main() {
  const auto started = grevir::interrupt::Application<GrevirApplication>::start();
  if (started.outcome != grevir::interrupt::SetupOutcome::success
      || started.disposition != grevir::interrupt::CallDisposition::initiated) {
    return 1;
  }
  example::controller.raise();
  example::controller.raise();
  if (example::ticks != 0
      || grevir::event::dispatch<GrevirApplication>(1) != 1) {
    return 2;
  }
  const auto repeated = grevir::interrupt::Application<GrevirApplication>::start();
  if (example::ticks != 1
      || repeated.outcome != grevir::interrupt::SetupOutcome::success
      || repeated.disposition != grevir::interrupt::CallDisposition::replayed) {
    return 2;
  }
  std::puts("PASS mock interrupt event elided and dispatched in main loop");
  return 0;
}
