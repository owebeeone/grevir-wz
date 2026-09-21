#include "fixtures.hpp"
using namespace timer_prototype;
using namespace timer_prototype::fixtures;

template <int Case>
consteval auto probe() {
  auto p = greedy;
  if constexpr (Case == 1) { p.requests[1].key = p.requests[0].key; }
  if constexpr (Case == 2) {
    p.candidates[1].timer = 1; p.candidates[1].endpoints[0].channel = 11;
  }
  if constexpr (Case == 3) { p.requests[0].config.frequency = FrequencyWindow::around({2000, 1}); }
  if constexpr (Case == 4) { p.candidates[1].key = 1; }
  if constexpr (Case == 5) { return compile(p, 1); }
  if constexpr (Case == 6) {
    return compile(Problem{p.requests, p.candidates, p.resources, std::array{1u}});
  }
  if constexpr (Case == 7) {
    using Contradictory = PwmRequest<"pwm", Frequency<Hertz<1000>, WithinPpm<10'000>>,
      For<Target::avr, Frequency<Hertz<1020>, Exact>>, Pin<101>, DutyStepAtMost<1, 256>>;
    p.requests = requests<Target::avr, Instance<"a", Contradictory>, B>();
  }
  return compile(p);
}

constexpr auto result = probe<PROTOTYPE_CASE>();
constexpr std::array expected{Status::success, Status::duplicate_identity, Status::conflict,
  Status::no_candidate, Status::model_error, Status::exhausted, Status::reserved, Status::invalid_request};
static_assert(result.diagnostic.status == expected[PROTOTYPE_CASE], "UNEXPECTED_PROTOTYPE_STATUS");
static_assert(PROTOTYPE_CASE != 7 || result.diagnostic.detail == static_cast<unsigned>(ConfigError::conflict),
  "UNEXPECTED_PROTOTYPE_STATUS");
constexpr bool enforced = [] {
  require_success<result.diagnostic.status>();
  return true;
}();
