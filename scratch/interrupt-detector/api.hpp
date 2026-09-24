#pragma once

#if defined(GREVIR_IRQ_DISCOVERY)
template <class Event>
struct DetectorImpl {
  static constexpr unsigned binding_id = 0u;
};
#else
template <class Event>
struct DetectorImpl;
#endif

template <class Event, class Detector = DetectorImpl<Event>,
          unsigned BindingId = Detector::binding_id>
void on_interrupt() noexcept = delete;

struct MotorInstance {
  struct Timer {
    struct PeriodElapsed {
      static constexpr unsigned event_id = 1001u;
    };
  };

  static inline int ticks = 0;

  static void tick() noexcept {
    ++ticks;
  }
};

struct MappedButNoHandler {
  static constexpr unsigned event_id = 1002u;
};
