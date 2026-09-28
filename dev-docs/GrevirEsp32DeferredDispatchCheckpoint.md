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
up to four records per pass. The board declares four records; the generated
plan derives a stable policy identity from the lock and context identities.
`Stream` uses the same ESP32 queue
policy when selected; the example retains the default `Elide` route.

The shared queue chooses `EventLock::TaskGuard` for setup, loop publication,
dispatch, and flag inspection, and `EventLock::IsrGuard` for interrupt
publication. On ESP32 both guard types enter one static `portMUX_TYPE` with
the matching task or ISR FreeRTOS critical-section API. Metadata changes occur
under that lock; callbacks run after it is released. No FreeRTOS blocking,
notification, or yielding operation occurs inside a critical section. Mock
and AVR retain their previous locking behavior through two aliases to their
existing guard.

The ESP32 main-loop context is specific to an application and binds to the task
calling application startup,
which is Arduino's `loopTask` when startup is called from `setup()`. A call to
`dispatch` from a different task returns zero without consuming records.
Publication from other tasks is permitted. This first slice polls from
`loop()` without a notification or sleeping dispatcher; latency is therefore
bounded by application loop cadence, not by the timer alone. The current
ESP32 timer entry is cache-dependent and not claimed IRAM safe.
All selected deferred plans require an explicit context policy; mock and AVR
use an unrestricted one. Preparing an active or currently dispatching queue
fails without rebinding or discarding its records. The startup runner reports
`event_context_failed` if it did not acquire the queue.

Validation: the macOS native CTest suite passed all 192 cases, including
cross-thread publication, foreign-dispatch rejection, active reprepare
rejection, and stopped-queue ownership transfer. The generated mock example
passed its CMake round trip. On Raspberry Pi, a temporary copy of the current
sources passed the staged Uno and classic ESP32 Arduino compile/link builds;
the existing remote Grevir checkout was not changed. A deliberately modified
ESP32 board with its context alias removed failed in the target probe with
`GREVIR_EVENT_CONTEXT_UNAVAILABLE`.
The policy identity now length-prefixes its lock and context components. Two
component pairs that previously collapsed to the same policy produced distinct
generated plans and fingerprints, and compiling one generated binding against
the other live policy failed with `GREVIR_IRQ_STALE_EVENT_CONTEXT_POLICY`.
After that correction, the native suite again passed all 192 cases, the clean
mock generator round trip passed, and both staged Pi Arduino builds passed.

Target compilation proves toolchain integration but not cross-core timing or
physical interrupt behavior. No claim is made for S2/S3 or other ESP32
families, named FreeRTOS contexts, software-originated events, or hardware
validation.
