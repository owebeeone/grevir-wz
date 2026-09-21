#include "fixtures.hpp"
#include <iostream>
#include <stdexcept>

using namespace timer_prototype;
using namespace timer_prototype::fixtures;

int main() {
  std::size_t checked = 0;
  // Independent oracle: enumerate all 27 timer assignments, using only the edge
  // mask and exclusive timer rule, without solver filtering/comparison helpers.
  for (unsigned mask = 0; mask < 512; ++mask) {
    bool feasible = false;
    std::array<unsigned, 3> expected{};
    for (unsigned x = 0; x < 3 && !feasible; ++x) {
      for (unsigned y = 0; y < 3 && !feasible; ++y) {
        for (unsigned z = 0; z < 3 && !feasible; ++z) {
          if (x != y && x != z && y != z && (mask & (1u << x))
              && (mask & (1u << (3 + y))) && (mask & (1u << (6 + z)))) {
            expected = {x + 1, y + 4, z + 7}; feasible = true;
          }
        }
      }
    }
    auto reqs = requests<Target::avr, A, B, C>();
    std::array<Candidate, 9> choices{};
    constexpr std::array<Key, 3> keys{a, b, c};
    Diagnostic expected_diagnostic;
    if (!feasible) {
      expected_diagnostic.status = Status::conflict;
      for (unsigned r = 0; r < 3; ++r) {
        if (((mask >> (r * 3)) & 7u) == 0) {
          expected_diagnostic = {Status::no_candidate, keys[r], 0};
          break;
        }
      }
    }
    for (unsigned r = 0; r < 3; ++r) {
      for (unsigned timer = 0; timer < 3; ++timer) {
        const unsigned i = r * 3 + timer;
        choices[i] = choice(i + 1, keys[r], timer + 1, 101 + r);
        if ((mask & (1u << i)) == 0) {
          // An individually unrealizable edge fails exact frequency filtering.
          choices[i].frequency = {1001, 1}; choices[i].configuration = 2;
        }
      }
    }
    const auto reference = solve(Problem{reqs, choices, resources, std::array<unsigned, 0>{}});
    do {
      for (unsigned reverse = 0; reverse < 2; ++reverse) {
        auto inventory = resources;
        if (reverse != 0) {
          std::reverse(choices.begin(), choices.end());
          std::reverse(inventory.begin(), inventory.end());
        }
        const auto result = solve(Problem{reqs, choices, inventory, std::array<unsigned, 0>{}});
        if (result.ok() != feasible || (feasible && result.candidates != expected)
            || result.diagnostic != expected_diagnostic
            || result.diagnostic != reference.diagnostic || result.candidates != reference.candidates
            || result.visited != reference.visited) { throw std::runtime_error("oracle/permutation mismatch"); }
        if (!result.ok() && result.candidates != std::array{0u, 0u, 0u}) {
          throw std::runtime_error("failure leaked partial assignment");
        }
        ++checked;
        if (reverse != 0) { std::reverse(choices.begin(), choices.end()); }
      }
    } while (std::next_permutation(reqs.begin(), reqs.end(), [](const auto& x, const auto& y) { return x.key < y.key; }));
  }
  std::cout << checked << " oracle/permutation comparisons across all 512 three-request/three-timer graphs passed.\n";
}
