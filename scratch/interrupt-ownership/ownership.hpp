#pragma once

#include <grevir/core/allocated_application.hpp>

namespace probe {

template <unsigned SourceId>
struct InterruptSource {};

inline unsigned registrations = 0;

template <typename Requests, typename FixedClaims>
struct Allocation {
  struct Plan {
    constexpr bool ok() const { return true; }
  };
  inline static constexpr Plan plan{};
  using Claims = ardo::ResourceClaim<>;

  static void setup() { ++registrations; }
};

struct Backend {
  template <typename Requests, typename FixedClaims>
  using Allocate = Allocation<Requests, FixedClaims>;
};

template <typename Allocation>
struct UartOwner : ardo::ModuleBase<> {};

template <typename Allocation>
struct FirstConsumer : ardo::ModuleBase<> {};

template <typename Allocation>
struct SecondConsumer : ardo::ModuleBase<> {};

template <typename Allocation>
struct CompetingOwner : ardo::ModuleBase<> {};

template <typename Allocation>
struct OtherSourceOwner : ardo::ModuleBase<> {};

using Owner = grevir::RequestedModule<setl::TypeArgs<>, UartOwner,
    ardo::ResourceClaim<InterruptSource<0>>>;
using ConsumerA = grevir::RequestedModule<setl::TypeArgs<>, FirstConsumer,
    ardo::ResourceClaim<>, Owner>;
using ConsumerB = grevir::RequestedModule<setl::TypeArgs<>, SecondConsumer,
    ardo::ResourceClaim<>, Owner>;
using Rival = grevir::RequestedModule<setl::TypeArgs<>, CompetingOwner,
    ardo::ResourceClaim<InterruptSource<0>>>;
using DistinctOwner = grevir::RequestedModule<setl::TypeArgs<>, OtherSourceOwner,
    ardo::ResourceClaim<InterruptSource<1>>>;

} // namespace probe
