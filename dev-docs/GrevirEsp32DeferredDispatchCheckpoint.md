# Classic ESP32 deferred-dispatch checkpoint

Status: implementation draft awaiting review. Scope is the classic ESP32 Dev
Module with Arduino-ESP32 3.3.11 and the existing Timer Group 0 / Timer 0
interrupt example. Silicon validation remains deferred.

The existing application closure, timer candidate, interrupt probe, generated
binding, and startup order remain authoritative. A specialization of
`on_event<PeriodElapsed>()` activates the event without an additional user
binding list. Its default route is `MainLoop`/`Elide`. Startup prepares the
queue before enabling the timer source; a failed startup stops the queue.
The generated callback posts through `post_from_isr`, and `loop()` dispatches
up to four records per pass. The board declares four records and a stable
policy identity in the generated plan. `Stream` uses the same ESP32 queue
policy when selected; the example retains the default `Elide` route.

The shared queue chooses `EventLock::TaskGuard` for setup, loop publication,
dispatch, and flag inspection, and `EventLock::IsrGuard` for interrupt
publication. On ESP32 both guard types enter one static `portMUX_TYPE` with
the matching task or ISR FreeRTOS critical-section API. Metadata changes occur
under that lock; callbacks run after it is released. No FreeRTOS blocking,
notification, or yielding operation occurs inside a critical section. Mock
and AVR retain their previous locking behavior through two aliases to their
existing guard.

The ESP32 main-loop context binds to the task calling application startup,
which is Arduino's `loopTask` when startup is called from `setup()`. A call to
`dispatch` from a different task returns zero without consuming records.
Publication from other tasks is permitted. This first slice polls from
`loop()` without a notification or sleeping dispatcher; latency is therefore
bounded by application loop cadence, not by the timer alone. The current
ESP32 timer entry is cache-dependent and not claimed IRAM safe.

Validation targets are shared host queue tests (including cross-thread
publication and foreign-dispatch rejection), mock generator round trip,
AVR staged compile/link regression, and classic ESP32 staged compile/link.
Target compilation proves toolchain integration but not cross-core timing or
physical interrupt behavior. No claim is made for S2/S3 or other ESP32
families, named FreeRTOS contexts, software-originated events, or hardware
validation.
