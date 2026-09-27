#include "../../docs/examples/interrupt-avr/avr_app_base.hpp"

template <>
struct grevir::event::RouteFor<avr_app::PeriodElapsed> {
  using Context = grevir::event::MainLoop;
  using Delivery = grevir::event::Stream;
};

template <>
inline void grevir::on_event<avr_app::PeriodElapsed>() noexcept {
  avr_app::grevir_irq_test_ticks =
    static_cast<unsigned char>(avr_app::grevir_irq_test_ticks + 1);
}

void stream_queue_probe() noexcept {
  grevir::event::prepare<GrevirApplication>();
  (void)grevir::event::post_from_isr<GrevirApplication, avr_app::PeriodElapsed>();
  (void)grevir::event::post_from_isr<GrevirApplication, avr_app::PeriodElapsed>();
  (void)grevir::event::dispatch<GrevirApplication>(2);
}
