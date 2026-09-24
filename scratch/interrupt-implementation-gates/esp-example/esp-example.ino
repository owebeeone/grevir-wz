#include <GrevirBase.h>
#include <GrevirTime.h>
#include <GrevirCore.h>
#include <GrevirPeripherals.h>
#include <GrevirArduino.h>
#include <GrevirArduinoESP32.h>
#include "esp_app.hpp"

void setup() {
  const auto result = grevir::interrupt::Application<GrevirApplication>::start();
  if (result.outcome != grevir::interrupt::SetupOutcome::success) {
    Serial.begin(115200);
    Serial.printf("Grevir IRQ startup failed: %u (origin %u)\n",
      static_cast<unsigned>(result.outcome),
      static_cast<unsigned>(result.originating_failure));
    for (;;) { delay(1000); }
  }
}

void loop() {}
