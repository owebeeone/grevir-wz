#pragma once
#include <cstdint>
#include <numeric>

namespace timer_prototype {

// Deliberately bounded HOST-only arithmetic. Values up to one million make the
// three-factor ppm comparison fit uint64_t (at most 10^18). Not firmware code.
struct Ratio {
  std::uint32_t numerator = 0;
  std::uint32_t denominator = 1;
  constexpr bool valid() const {
    return numerator > 0 && numerator <= 1'000'000
      && denominator > 0 && denominator <= 1'000'000;
  }
  constexpr Ratio normalized() const {
    const auto divisor = std::gcd(numerator, denominator);
    return {numerator / divisor, denominator / divisor};
  }
  constexpr bool operator==(const Ratio&) const = default;
};

constexpr bool at_most(Ratio a, Ratio b) {
  return std::uint64_t{a.numerator} * b.denominator
    <= std::uint64_t{b.numerator} * a.denominator;
}

constexpr bool within(Ratio actual, Ratio requested, std::uint32_t ppm) {
  const auto a = std::uint64_t{actual.numerator} * requested.denominator;
  const auto b = std::uint64_t{requested.numerator} * actual.denominator;
  const auto difference = a > b ? a - b : b - a;
  return difference * 1'000'000 <= b * ppm;
}

} // namespace timer_prototype
