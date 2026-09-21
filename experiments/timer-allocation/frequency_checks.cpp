#include "fixtures.hpp"
#include <iostream>
#include <stdexcept>

using namespace timer_prototype;
using namespace timer_prototype::fixtures;

using CommonRate = Frequency<Hertz<1000>, WithinPpm<10'000>>; // +/- 1%.
using Combined = PwmRequest<"pwm", CommonRate, Pin<101>, DutyStepAtMost<1, 256>,
  For<Target::avr, Frequency<Hertz<1000>, Exact>>,
  For<Target::esp32, Frequency<Hertz<1005>, Exact>>>;
using Reversed = PwmRequest<"pwm",
  For<Target::esp32, Frequency<Hertz<1005>, Exact>>,
  For<Target::avr, Frequency<Hertz<1000>, Exact>>,
  DutyStepAtMost<1, 256>, Pin<101>, CommonRate>;

constexpr auto avr_config = request<Target::avr, Instance<"a", Combined>>().config;
constexpr auto esp_config = request<Target::esp32, Instance<"a", Combined>>().config;
static_assert(avr_config.error == ConfigError::none && esp_config.error == ConfigError::none);
static_assert(avr_config.frequency == FrequencyWindow::around({1000, 1}));
static_assert(esp_config.frequency == FrequencyWindow::around({1005, 1}));
static_assert(avr_config.frequency == request<Target::avr, Instance<"a", Reversed>>().config.frequency);
static_assert(esp_config.frequency == request<Target::esp32, Instance<"a", Reversed>>().config.frequency);

constexpr auto candidates = [] {
  auto exact = choice(1, a, 1, 101);
  auto other = choice(2, a, 1, 101);
  other.configuration = 2; other.frequency = {1005, 1};
  return std::array{exact, other};
}();
constexpr auto avr_result = compile(Problem{requests<Target::avr, Instance<"a", Combined>>(),
  candidates, resources, std::array<unsigned, 0>{}});
constexpr auto esp_result = compile(Problem{requests<Target::esp32, Instance<"a", Combined>>(),
  candidates, resources, std::array<unsigned, 0>{}});
static_assert(avr_result.ok() && avr_result.candidates == std::array{1u});
static_assert(esp_result.ok() && esp_result.candidates == std::array{2u});

using Contradictory = PwmRequest<"pwm", CommonRate, Pin<101>, DutyStepAtMost<1, 256>,
  For<Target::avr, Frequency<Hertz<1020>, Exact>>>;
using ContradictoryReversed = PwmRequest<"pwm", For<Target::avr, Frequency<Hertz<1020>, Exact>>,
  Pin<101>, DutyStepAtMost<1, 256>, CommonRate>;
constexpr auto bad = compile(Problem{requests<Target::avr, Instance<"a", Contradictory>>(),
  candidates, resources, std::array<unsigned, 0>{}});
constexpr auto bad_reversed = compile(Problem{requests<Target::avr, Instance<"a", ContradictoryReversed>>(),
  candidates, resources, std::array<unsigned, 0>{}});
static_assert(bad.diagnostic == Diagnostic{Status::invalid_request, a, static_cast<unsigned>(ConfigError::conflict)});
static_assert(bad.diagnostic == bad_reversed.diagnostic && bad.candidates == std::array{0u});
// The AVR contradiction is inactive on ESP32, leaving the common tolerance.
static_assert(request<Target::esp32, Instance<"a", Contradictory>>().config.frequency
  == FrequencyWindow::around({1000, 1}, 10'000));

constexpr auto touching = FrequencyWindow::around({1, 3}, 500'000)
  .intersect(FrequencyWindow::around({2, 3}, 250'000));
static_assert(touching == FrequencyWindow::around({1, 2}));
static_assert(touching.contains({1, 2}));
static_assert(!touching.contains({499'999, 1'000'000}));
static_assert(!touching.contains({500'001, 1'000'000}));

// Overlap [999.975, 1010] cannot be represented as either original +/- tolerance.
constexpr auto overlap = FrequencyWindow::around({1000, 1}, 10'000)
  .intersect(FrequencyWindow::around({1005, 1}, 5'000));
static_assert(overlap.contains({39'999, 40}));
static_assert(overlap.contains({1010, 1}));
static_assert(!overlap.contains({499'987, 500})); // 999.974, immediately below.
static_assert(!overlap.contains({505'001, 500})); // 1010.002, immediately above.
static_assert(overlap.lower == FrequencyBound{999'975'000, 1});
static_assert(overlap.upper == FrequencyBound{1'010'000'000, 1});

using Fractional = PwmRequest<"pwm", Frequency<Hertz<1, 3>, WithinPpm<500'000>>,
  Frequency<Hertz<2, 3>, WithinPpm<250'000>>, Pin<101>, DutyStepAtMost<1, 256>>;
static_assert(request<Target::avr, Instance<"a", Fractional>>().config.frequency == touching);

constexpr auto maximum = FrequencyWindow::around({1'000'000, 1}, 1'000'000);
static_assert(maximum.lower == FrequencyBound{0, 1});
static_assert(maximum.upper == FrequencyBound{2'000'000'000'000, 1});
static_assert(maximum.contains({1, 1'000'000}) && maximum.contains({1'000'000, 1}));
static_assert(!maximum.contains({0, 1})); // PWM candidates must have positive rates.
constexpr auto tiny = FrequencyWindow::around({1, 1'000'000}, 1);
static_assert(tiny.contains({1, 1'000'000}));
static_assert(!tiny.contains({1, 999'999}));
static_assert(tiny.lower == FrequencyBound{999'999, 1'000'000});
static_assert(FrequencyWindow::around({2, 6}, 10'000) == FrequencyWindow::around({1, 3}, 10'000));
static_assert(!FrequencyWindow::around({1, 0}).valid());
static_assert(!FrequencyWindow::around({1, 1}, 1'000'001).valid());
static_assert(!FrequencyWindow::around({0, 1}).valid());
static_assert(!FrequencyWindow{}.contains({1000, 1}));

constexpr bool permutations() {
  const std::array overlapping{
    FrequencyWindow::around({1000, 1}, 10'000),
    FrequencyWindow::around({1005, 1}, 5'000),
    FrequencyWindow::around({1000, 1})
  };
  const std::array disjoint{
    FrequencyWindow::around({1000, 1}),
    FrequencyWindow::around({1001, 1}),
    FrequencyWindow::around({1000, 1}, 10'000)
  };
  auto expected_empty = disjoint[0].intersect(disjoint[1]);
  std::array<unsigned, 3> order{0, 1, 2};
  do {
    FrequencyWindow good, bad;
    for (auto i : order) {
      good = good.intersect(overlapping[i]); bad = bad.intersect(disjoint[i]);
    }
    if (good != FrequencyWindow::around({1000, 1}) || bad != expected_empty || !bad.empty()) { return false; }
  } while (std::next_permutation(order.begin(), order.end()));
  return true;
}
static_assert(permutations());

// A nonempty request interval without a hardware candidate is not a contradictory
// request. A shared timer must meet the intersection for every member too.
constexpr bool allocation_boundaries() {
  auto p = sharing();
  p.requests[0].config.frequency = FrequencyWindow::around({1000, 1}, 10'000);
  p.requests[1].config.frequency = FrequencyWindow::around({1005, 1});
  p.candidates[0].frequency = {1005, 1};
  if (!solve(p).ok()) { return false; }
  p.requests[1].config.frequency = FrequencyWindow::around({1020, 1});
  p.candidates[0].frequency = {1020, 1};
  if (solve(p).diagnostic.status != Status::no_candidate) { return false; }
  auto g = greedy;
  g.requests[0].config.frequency = FrequencyWindow::around({1, 2});
  return solve(g).diagnostic.status == Status::no_candidate;
}
static_assert(allocation_boundaries());

int main() {
  constexpr std::array rates{Ratio{1, 1}, Ratio{1, 3}, Ratio{2, 3}, Ratio{999, 1},
    Ratio{1000, 1}, Ratio{1001, 1}, Ratio{1, 1'000'000}, Ratio{999'999, 1'000'000},
    Ratio{1'000'000, 999'999}, Ratio{1'000'000, 1}, Ratio{999'999, 1}, Ratio{999'983, 999'979}};
  constexpr std::array tolerances{0u, 1u, 999u, 1000u, 10'000u, 500'000u, 1'000'000u};
  std::size_t checked = 0;
  for (const auto left : rates) {
    for (const auto left_ppm : tolerances) {
      for (const auto right : rates) {
        for (const auto right_ppm : tolerances) {
          const auto a = FrequencyWindow::around(left, left_ppm);
          const auto b = FrequencyWindow::around(right, right_ppm);
          const auto combined = a.intersect(b);
          if (combined != b.intersect(a) || combined != combined.intersect(a)
              || combined != combined.intersect(b)) { throw std::runtime_error("noncanonical intersection"); }
          for (const auto actual : rates) {
            // Independent expression: apply the original relative-error
            // inequalities separately, without constructing interval bounds.
            const bool expected = within(actual, left, left_ppm) && within(actual, right, right_ppm);
            if (combined.contains(actual) != expected) { throw std::runtime_error("frequency intersection mismatch"); }
            ++checked;
          }
        }
      }
    }
  }
  std::cout << checked << " interval memberships agree with independent relative-error inequalities.\n";
}
