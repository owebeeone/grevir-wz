#include <GrevirBase.h>
#include <GrevirTime.h>
#include <GrevirCore.h>
#include <GrevirPeripherals.h>
#include <GrevirRegisters.h>
#include <GrevirAVR.h>
#include "avr_app.hpp"

namespace avr_app {
extern "C" {
volatile unsigned char grevir_irq_test_ticks = 0;
}
volatile unsigned char grevir_irq_start_failure = 0;
}

void setup() {
  const auto result = grevir::interrupt::Application<GrevirApplication>::start();
  if (result.outcome != grevir::interrupt::SetupOutcome::success) {
    avr_app::grevir_irq_start_failure = static_cast<unsigned char>(result.outcome);
    for (;;) {}
  }
}

void loop() {}
