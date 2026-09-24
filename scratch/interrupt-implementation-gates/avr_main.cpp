#include "avr_app.hpp"

namespace avr_app {
extern "C" {
volatile unsigned char grevir_irq_test_ticks = 0;
}
}

int main() {
  grevir::avr::atmega328p::Timer1Overflow::mask();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0xfffeu;
  TIFR1 = _BV(TOV1);
  if (!grevir::interrupt::detail::install_bindings<GrevirApplication>()) { return 1; }
  TCCR1B = _BV(CS10);
  grevir::avr::atmega328p::Timer1Overflow::enable_preserving_pending();
  sei();
  while (avr_app::grevir_irq_test_ticks == 0) {}
  return 0;
}
