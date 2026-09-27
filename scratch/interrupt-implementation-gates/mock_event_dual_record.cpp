#include "mock_event_app.hpp"
#include <grevir/interrupt/probe_section.hpp>

template <>
inline void grevir::on_interrupt<mock_app::PeriodElapsed>() noexcept {}

GREVIR_EMIT_IRQ_PROBE(GrevirApplication);
