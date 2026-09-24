#include "mock_decl_only.hpp"
#include <grevir/interrupt/start.hpp>

int main() {
  return grevir::interrupt::Application<GrevirApplication>::start().outcome
      == grevir::interrupt::SetupOutcome::success ? 0 : 1;
}
