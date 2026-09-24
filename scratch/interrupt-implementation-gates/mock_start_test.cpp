#include "mock_app.hpp"
#include <grevir/interrupt/start.hpp>
#include <atomic>
#include <thread>

namespace {
using App = grevir::interrupt::Application<GrevirApplication>;
using Policy = grevir::test::HostStartPolicy<GrevirApplication>;
using Outcome = grevir::interrupt::SetupOutcome;
using Disposition = grevir::interrupt::CallDisposition;

std::atomic<bool> entered{false};
std::atomic<bool> release{false};

void wait_in_configure() noexcept {
  entered.store(true, std::memory_order_release);
  while (!release.load(std::memory_order_acquire)) { std::this_thread::yield(); }
}

void reset() {
  Policy::reset_for_test();
  mock_app::controller.reset();
  mock_app::ticks = 0;
  mock_app::reraising = false;
  mock_app::configure_ok = true;
  mock_app::pending_ok = true;
  mock_app::cleanup_ok = true;
  mock_app::configure_hook = nullptr;
}

bool failure(Outcome requested, bool cleanup_succeeds) {
  const auto result = App::start();
  const auto expected = cleanup_succeeds ? requested : Outcome::cleanup_failed;
  const auto replayed = App::start();
  return result.outcome == expected && result.originating_failure == requested
    && result.disposition == Disposition::initiated
    && replayed.outcome == expected && replayed.originating_failure == requested
    && replayed.disposition == Disposition::replayed
    && !mock_app::controller.enabled();
}
}

int main() {
  reset();
  mock_app::configure_ok = false;
  if (!failure(Outcome::configuration_failed, true)) { return 1; }

  reset();
  mock_app::controller.reject_install(true);
  if (!failure(Outcome::registration_failed, true)) { return 2; }

  reset();
  mock_app::pending_ok = false;
  if (!failure(Outcome::pending_policy_failed, true)) { return 3; }

  reset();
  mock_app::configure_ok = false;
  mock_app::cleanup_ok = false;
  if (!failure(Outcome::configuration_failed, false)) { return 4; }

  reset();
  mock_app::controller.raise();
  const auto pending = App::start();
  if (pending.outcome != Outcome::success || mock_app::ticks != 1
      || mock_app::controller.pending()) { return 5; }

  reset();
  mock_app::reraising = true;
  if (App::start().outcome != Outcome::success) { return 6; }
  mock_app::controller.raise();
  if (mock_app::ticks != 2 || mock_app::controller.pending()) { return 7; }

  reset();
  entered.store(false);
  release.store(false);
  mock_app::configure_hook = wait_in_configure;
  grevir::interrupt::StartResult first{};
  grevir::interrupt::StartResult second{};
  std::thread initiator([&] { first = App::start(); });
  while (!entered.load(std::memory_order_acquire)) { std::this_thread::yield(); }
  std::thread waiter([&] { second = App::start(); });
  while (Policy::waiting_for_test() == 0) { std::this_thread::yield(); }
  release.store(true, std::memory_order_release);
  initiator.join();
  waiter.join();
  if (first.outcome != Outcome::success || first.disposition != Disposition::initiated
      || second.outcome != Outcome::success || second.disposition != Disposition::waited) {
    return 8;
  }
  return 0;
}
