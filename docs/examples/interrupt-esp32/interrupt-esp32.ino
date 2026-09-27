#include <GrevirBase.h>
#include <GrevirTime.h>
#include <GrevirCore.h>
#include <GrevirPeripherals.h>
#include <GrevirArduino.h>
#include <GrevirArduinoESP32.h>
#include "esp_app.hpp"

void setup() {
  Serial.begin(115200);
  const auto result = grevir::interrupt::Application<GrevirApplication>::start();
  esp_app::started = result.outcome == grevir::interrupt::SetupOutcome::success;
  if (!esp_app::started) {
    Serial.printf("Grevir IRQ startup failed: %u (origin %u)\n",
      static_cast<unsigned>(result.outcome),
      static_cast<unsigned>(result.originating_failure));
  }
}

void loop() {
  if (esp_app::started) {
    grevir::event::dispatch<GrevirApplication>(4);
    if (grevir::event::overrun<GrevirApplication>()) {
      Serial.println("Grevir event queue overrun");
      grevir::event::clear_overrun<GrevirApplication>();
    }
  }
}
