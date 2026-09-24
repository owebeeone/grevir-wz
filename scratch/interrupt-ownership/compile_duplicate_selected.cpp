#include "ownership.hpp"

namespace probe {

template <typename Requests, typename FixedClaims>
struct DuplicateAllocation {
  struct Plan {
    constexpr bool ok() const { return true; }
  };
  inline static constexpr Plan plan{};
  using Claims = ardo::ResourceClaim<InterruptSource<0>, InterruptSource<0>>;

  static void setup() {}
};

struct DuplicateBackend {
  template <typename Requests, typename FixedClaims>
  using Allocate = DuplicateAllocation<Requests, FixedClaims>;
};

} // namespace probe

using Application = grevir::AllocatedApplication<probe::DuplicateBackend>;

int main() {
  Application::runSetup();
}
