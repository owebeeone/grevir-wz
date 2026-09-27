#pragma once

#include "poc.hpp"

namespace grevir_poc {

struct PendingMark {
  bool value = false;
};

template <class Event>
struct MockEventState {
  inline static PendingMark pending{};
};

// Mock-only backend. A target backend must serialize each post with dequeue.
class MockBackend {
public:
  static constexpr unsigned capacity = 2;

  template <class Event>
  static bool post_from_isr(void (*invoke)(void*) noexcept) noexcept {
    PendingMark& mark = MockEventState<Event>::pending;
    if (mark.value) {
      return false;
    }
    if (count_ == capacity) {
      return false;
    }
    records_[write_] = {invoke, &mark};
    write_ = (write_ + 1) % capacity;
    ++count_;
    mark.value = true;
    return true;
  }

  static bool dispatch_one() noexcept {
    if (count_ == 0) {
      return false;
    }
    const DispatchRecord record = records_[read_];
    read_ = (read_ + 1) % capacity;
    --count_;
    static_cast<PendingMark*>(record.argument)->value = false;
    record.invoke(record.argument);
    return true;
  }

  static unsigned queued() noexcept {
    return count_;
  }

private:
  inline static DispatchRecord records_[capacity]{};
  inline static unsigned read_ = 0;
  inline static unsigned write_ = 0;
  inline static unsigned count_ = 0;
};

} // namespace grevir_poc
