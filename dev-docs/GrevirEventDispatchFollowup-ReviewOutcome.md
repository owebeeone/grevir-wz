# Deferred event dispatch follow-up — review outcome

Status: **accepted at root `2f25854834f83a2f9fbb0372a352d5bb5f1a1be0`, Core `e37470b5e6ffa628e0788727c56704c394a46a07` after the [Code](GrevirEventDispatchFollowup-ReviewCode.md), [State](GrevirEventDispatchFollowup-ReviewState.md), and [Surface](GrevirEventDispatchFollowup-ReviewSurface.md) axes reported GO; this accepts the follow-up remediation scope only.** The unchanged Test Support, Peripherals, and AVR revisions were `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec`, `4612cecdc99c22f326a0c1498846fea34c0c596c`, and `5f4feffa15110aac9bf27f0bc17c2ce668b003d6`. The three reports were independent, peer-blind, and read-only on that exact clean tuple.

The previous [NO-GO](GrevirEventDispatchCheckpoint-ReviewOutcome.md) remains the outcome of its earlier tuple. This new checkpoint closes Code P2-1 by comparing the strict build's complete live handler demand against the generated snapshot, including zero-to-one and one-to-two additions. It closes State P2-1 by validating the original board queue capacity before conversion, only when deferred delivery is selected. The selected capacity, not the 2048 ceiling, determines queue storage and index width. The three earlier P3 documentation corrections are present. No reviewer reported a new blocker or a new architectural root cause in this follow-up.

Validation on the accepted implementation tuple:

| Environment | Result |
| --- | --- |
| macOS, Clang | Probe/plan/emit/strict mock round trip, including stale-demand and invalid/valid capacity cases, passed; generated mock CMake target and its CTest passed; native suite passed 189/189 CTests. |
| Raspberry Pi, GCC | The same expanded mock round trip passed. AVR GCC 14.2 accepted a 2048 capacity declaration and rejected `65537UL` with `GREVIR_EVENT_CAPACITY_OUT_OF_RANGE`. |
| Raspberry Pi, AVR GCC + simavr | The staged Uno Timer1 sketch compiled and linked (1178 bytes flash, 46 bytes globals); simavr observed one handler firing at cycle 66166. |
| Raspberry Pi, classic ESP32 Dev Module / Arduino-ESP32 3.3.11 | The staged direct-interrupt sketch compiled and linked (270188 bytes flash, 22148 bytes globals). |
| Win11, MSVC | Native probe/JSON/generated mock dispatch, `on_event` activation, strict-only added-handler rejection, and capacity-bound checks passed through `check_msvc.bat`. |

These checks establish compiler, generator, host runtime, and simulated AVR behavior for the named paths. Physical silicon validation remains on hold. Inherited board-owned peripheral setup, `Stream`, other event contexts, and ESP32 deferred queue synchronization remain outside this accepted checkpoint.
