#include <GrevirBase.h>
#include <GrevirTime.h>
#include <GrevirCore.h>
#include <GrevirPeripherals.h>
#include <GrevirArduino.h>
#include <GrevirArduinoESP32.h>
#include "esp_app.hpp"

namespace {
bool startup_finished = false;

void attempt_start() {
  const auto result = grevir::interrupt::Application<GrevirApplication>::start();
  if (result.outcome == grevir::interrupt::SetupOutcome::in_progress) {
    return;
  }
  startup_finished = true;
  esp_app::started = result.outcome == grevir::interrupt::SetupOutcome::success;
  if (!esp_app::started) {
    Serial.printf("Grevir IRQ startup failed: %u (origin %u)\n",
      static_cast<unsigned>(result.outcome),
      static_cast<unsigned>(result.originating_failure));
  }
}
}

void setup() {
  Serial.begin(115200);
  attempt_start();
}

void loop() {
  if (!startup_finished) {
    attempt_start();
  }
  if (esp_app::started) {
    grevir::event::dispatch<GrevirApplication>(4);
    if (grevir::event::overrun<GrevirApplication>()) {
      Serial.println("Grevir event queue overrun");
      grevir::event::clear_overrun<GrevirApplication>();
    }
  }
}
