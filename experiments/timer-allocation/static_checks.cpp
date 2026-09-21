#include "fixtures.hpp"
using namespace timer_prototype;
using namespace timer_prototype::fixtures;

constexpr auto plan = compile(greedy);
static_assert(plan.ok() && plan.candidates == std::array{2u, 3u});
static_assert(plan.visited == 4); // Failed T1/T1, then successful T2/T1.
static_assert(compile(greedy, 3).diagnostic.status == Status::exhausted);
static_assert(compile(greedy, 4).ok());
static_assert(compile(greedy, 0).candidates == std::array{0u, 0u});

constexpr bool permutation() {
  auto p = greedy;
  std::reverse(p.requests.begin(), p.requests.end());
  std::reverse(p.candidates.begin(), p.candidates.end());
  std::reverse(p.resources.begin(), p.resources.end());
  const auto result = solve(p);
  return result.diagnostic == plan.diagnostic && result.candidates == plan.candidates
    && result.requests == plan.requests && result.visited == plan.visited;
}
static_assert(permutation());

constexpr auto avr_request = request<Target::atmega328p, Instance<"motor", Portable>>();
constexpr auto esp_request = request<Target::esp32, Instance<"motor", Portable>>();
static_assert(avr_request.config.pin == 101 && avr_request.config.source == Source::icr);
static_assert(esp_request.config.pin == 102 && esp_request.config.source == Source::apb);
static_assert(avr_request.config.frequency == esp_request.config.frequency);
static_assert(avr_request.config.error == ConfigError::none && esp_request.config.error == ConfigError::none);

constexpr auto portable_candidates = [] {
  auto avr = choice(1, {"motor", "pwm"}, 1, 101);
  auto esp = choice(2, {"motor", "pwm"}, 2, 102);
  esp.source = Source::apb;
  return std::array{avr, esp};
}();
constexpr auto avr_plan = compile(Problem{requests<Target::atmega328p, Instance<"motor", Portable>>(),
  portable_candidates, resources, std::array<unsigned, 0>{}});
constexpr auto esp_plan = compile(Problem{requests<Target::esp32, Instance<"motor", Portable>>(),
  portable_candidates, resources, std::array<unsigned, 0>{}});
static_assert(avr_plan.ok() && avr_plan.candidates == std::array{1u});
static_assert(esp_plan.ok() && esp_plan.candidates == std::array{2u});
static_assert(request<Target::avr, Instance<"motor", PoisonOnEsp>>().config.error == ConfigError::none);
static_assert(request<Target::avr, Instance<"motor", LazyOnEsp>>().config.error == ConfigError::none);
constexpr auto poison_plan = compile(Problem{requests<Target::avr, Instance<"motor", LazyOnEsp>>(),
  portable_candidates, resources, std::array<unsigned, 0>{}});
static_assert(poison_plan.candidates == avr_plan.candidates && poison_plan.ok());
static_assert(request<Target::esp32, Instance<"motor", PoisonOnEsp>>().config.error != ConfigError::none);

using Misplaced = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<101>,
  For<Target::esp32, avr::FastPwm>>;
static_assert(request<Target::esp32, Instance<"x", Misplaced>>().config.error == ConfigError::unsupported_option);
using Overlapping = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<101>,
  For<Target::avr, avr::FastPwm>, For<Target::atmega328p, avr::PhaseCorrectPwm>>;
static_assert(request<Target::avr, Instance<"x", Overlapping>>().config.error == ConfigError::none);
static_assert(request<Target::atmega328p, Instance<"x", Overlapping>>().config.error == ConfigError::conflict);

using MultipleRates = PwmRequest<"pwm", Frequency<Hertz<1000>, Exact>,
  Frequency<Hertz<1000>, WithinPpm<1000>>, DutyStepAtMost<1, 256>, Pin<101>>;
using MultipleRatesReversed = PwmRequest<"pwm", Frequency<Hertz<1000>, WithinPpm<1000>>,
  Frequency<Hertz<1000>, Exact>, DutyStepAtMost<1, 256>, Pin<101>>;
static_assert(request<Target::avr, Instance<"x", MultipleRates>>().config.error == ConfigError::unsupported_combination);
static_assert(request<Target::avr, Instance<"x", MultipleRatesReversed>>().config.error == ConfigError::unsupported_combination);

constexpr bool duplicate_diagnostics() {
  auto p = greedy;
  p.requests[1].key = p.requests[0].key;
  p.requests[0].config.error = ConfigError::invalid_value;
  p.requests[1].config.error = ConfigError::unsupported_option;
  const auto original = solve(p);
  std::reverse(p.requests.begin(), p.requests.end());
  return original.diagnostic == solve(p).diagnostic
    && original.diagnostic == Diagnostic{Status::duplicate_identity, a, 2};
}
static_assert(duplicate_diagnostics());
using BadContents = PwmRequest<"pwm", MustNotValidate>;
constexpr auto duplicate_types = Problem{
  requests<Target::avr, A, Instance<"a", BadContents>>(), greedy.candidates, resources, std::array<unsigned, 0>{}
};
static_assert(compile(duplicate_types).diagnostic == Diagnostic{Status::duplicate_identity, a, 2});
static_assert(compile(sharing()).candidates == std::array{1u, 1u});
static_assert(compile(domains()).candidates == std::array{1u, 3u});

constexpr bool ownership_cases() {
  auto p = sharing();
  std::swap(p.candidates[0].endpoints[0], p.candidates[0].endpoints[1]);
  std::reverse(p.requests.begin(), p.requests.end());
  if (!solve(p).ok()) { return false; }
  const auto reserved = Problem{p.requests, p.candidates, p.resources, std::array{11u}};
  if (solve(reserved).diagnostic != Diagnostic{Status::reserved, a, 11}) { return false; }
  p.candidates[0].independent_duties = false;
  if (solve(p).diagnostic.status != Status::no_candidate) { return false; }
  p = sharing();
  p.candidates[0].endpoints[1].channel = 11;
  if (solve(p).diagnostic.status != Status::model_error) { return false; }
  p = sharing();
  p.candidates[0].endpoints[1].pin = 101;
  if (solve(p).diagnostic.status != Status::model_error) { return false; }
  p = sharing();
  p.requests[1].config.frequency = {2000, 1};
  return solve(p).diagnostic.status == Status::no_candidate;
}
static_assert(ownership_cases());

constexpr bool failure_cases() {
  auto p = greedy;
  p.candidates[1].timer = 1;
  p.candidates[1].endpoints[0].channel = 11;
  if (solve(p).diagnostic.status != Status::conflict) { return false; }
  p = greedy;
  p.candidates[1].key = 1;
  if (solve(p).diagnostic.status != Status::model_error) { return false; }
  p = greedy;
  p.resources[0].parent = 11;
  if (solve(p).diagnostic.status != Status::model_error) { return false; }
  p = greedy;
  p.requests[0].config.pin = 999;
  if (solve(p).diagnostic.status != Status::model_error) { return false; }
  auto d = domains();
  d.candidates[2].setting = 64;
  if (solve(d).diagnostic.status != Status::conflict) { return false; }
  return solve(Problem{d.requests, d.candidates, d.resources, std::array{201u}}).diagnostic.status == Status::reserved;
}
static_assert(failure_cases());

constexpr bool more_boundaries() {
  auto p = greedy;
  // A child reservation excludes the whole timer, even its other channel.
  p.candidates[0].endpoints[0].channel = 12;
  p.candidates[2].endpoints[0].channel = 12;
  if (solve(Problem{p.requests, p.candidates, p.resources, std::array{11u}}).diagnostic.status != Status::reserved) {
    return false;
  }
  // Equal physical pin identities conflict even on distinct timers.
  auto d = domains();
  d.requests[1].config.pin = 101;
  for (auto& candidate : d.candidates) {
    if (candidate.endpoints[0].request == b) { candidate.endpoints[0].pin = 101; }
  }
  if (solve(d).diagnostic.status != Status::conflict) { return false; }
  if (solve(Problem{p.requests, p.candidates, p.resources, std::array{1u, 11u}}).diagnostic.status != Status::model_error) {
    return false;
  }
  return true;
}
static_assert(more_boundaries());

static_assert(within({999, 1}, {1000, 1}, 1000));
static_assert(within({1001, 1}, {1000, 1}, 1000));
static_assert(!within({998, 1}, {1000, 1}, 1000));
static_assert(!within({1002, 1}, {1000, 1}, 1000));
static_assert(!within({999, 1}, {1000, 1}, 0));
static_assert(within({1'000'000, 1'000'000}, {1, 1}, 0));
static_assert(!at_most({1, 255}, {1, 256}));
static_assert(at_most({1, 1024}, {1, 256}));

constexpr auto empty = Problem{std::array<Request, 0>{}, std::array<Candidate, 0>{}, resources, std::array<unsigned, 0>{}};
static_assert(compile(empty, 0).ok());

// Practical compile-time envelope exercised here: eight independent requests,
// eight candidates, immediate solution. This is not a worst-case bound.
constexpr bool envelope() {
  std::array<Request, 8> reqs{};
  std::array<Candidate, 8> choices{};
  std::array<Resource, 24> hw{};
  constexpr std::array<std::string_view, 8> names{"a", "b", "c", "d", "e", "f", "g", "h"};
  for (unsigned i = 0; i < 8; ++i) {
    reqs[i] = {{names[i], "pwm"}, {{1000, 1}, 0, {1, 256}, 101 + i}};
    choices[i] = choice(i + 1, reqs[i].key, i + 1, 101 + i);
    hw[i * 3] = {i + 1, 0, Kind::timer};
    hw[i * 3 + 1] = {(i + 1) * 10 + 1, i + 1, Kind::channel};
    hw[i * 3 + 2] = {101 + i, 0, Kind::pin};
  }
  const auto result = solve(Problem{reqs, choices, hw, std::array<unsigned, 0>{}});
  return result.ok() && result.visited == 8;
}
static_assert(envelope());
