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
  const auto repeated = grevir::interrupt::Application<GrevirApplication>::start();
  if (example::ticks != 1
      || repeated.outcome != grevir::interrupt::SetupOutcome::success
      || repeated.disposition != grevir::interrupt::CallDisposition::replayed) {
    return 2;
  }
  std::puts("PASS mock interrupt handler called once");
  return 0;
}
