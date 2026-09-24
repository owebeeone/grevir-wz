#include <GrevirBase.h>
#include <GrevirTime.h>
#include <GrevirCore.h>
#include <GrevirPeripherals.h>
#include <GrevirArduino.h>
#include <GrevirArduinoESP32.h>
#include "esp_app.hpp"

void setup() {
  if (grevir::interrupt::detail::install_bindings<GrevirApplication>()) {
    grevir::arduino_esp32::TimerGroup0Timer0::enable_preserving_pending();
  }
}

void loop() {}
