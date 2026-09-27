#include <grevir/event/queue_capacity.hpp>

#if !defined(GREVIR_TEST_CAPACITY_VALUE)
#error GREVIR_TEST_CAPACITY_VALUE must name the selected queue capacity
#endif

struct Board {
  inline static constexpr auto event_queue_capacity = GREVIR_TEST_CAPACITY_VALUE;
};

static_assert(grevir::event::detail::QueueCapacity<Board>::value > 0);
