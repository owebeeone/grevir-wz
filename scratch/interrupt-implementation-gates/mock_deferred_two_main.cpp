#include "mock_deferred_two_app.hpp"
#include <grevir/interrupt/start.hpp>

int main() {
  const auto started = grevir::interrupt::Application<GrevirApplication>::start();
  if (started.outcome != grevir::interrupt::SetupOutcome::success) { return 1; }
  mock_deferred_two::controller_a.raise();
  mock_deferred_two::controller_b.raise();
#if defined(GREVIR_TEST_STREAM_B)
  if (grevir::event::dispatch<GrevirApplication>(1) != 1) { return 2; }
  mock_deferred_two::controller_b.raise();
  if (grevir::event::dispatch<GrevirApplication>(2) != 2) { return 3; }
  if (mock_deferred_two::calls_a != 1 || mock_deferred_two::calls_b != 2) {
    return 4;
  }
#else
  if (grevir::event::dispatch<GrevirApplication>(2) != 2) { return 2; }
  if (mock_deferred_two::calls_a != 1 || mock_deferred_two::calls_b != 1) {
    return 3;
  }
#endif
  return 0;
}
