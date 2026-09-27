# Main-loop Stream checkpoint — review outcome

Status: **accepted at root `723df9ba7bd0452cd661c6c9e8b3bd6df2b8e1a0`
and Core `6d09870d78b0628da4c6a0007c4a7d45e4ae5473` after
[Code](GrevirEventStreamCheckpoint-ReviewCode.md),
[State](GrevirEventStreamCheckpoint-ReviewState.md), and focused
[Surface re-review](GrevirEventStreamCheckpoint-ReviewSurface-2.md) reported
GO; this accepts `MainLoop`/`Stream` for the named mock and ATmega328P queue
path only.** Test Support, Peripherals, and AVR remained at
`d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec`,
`4612cecdc99c22f326a0c1498846fea34c0c596c`, and
`5f4feffa15110aac9bf27f0bc17c2ce668b003d6`.

The initial Surface [NO-GO](GrevirEventStreamCheckpoint-ReviewSurface.md)
identified one P2 contradiction: an Elide coalescing rule was stated as common
queue behavior. The [remediation](GrevirEventStreamCheckpoint-RemPlan.md)
qualified the behavior by route. The original Surface reviewer traced two
firings at capacities one and two on the revised, clean root tuple and closed
that blocker. Code and State reviewed the same unchanged implementation tuple
independently and reported GO. No new structural API defect was found.

Validation on the implementation checkpoint:

| Environment | Result |
| --- | --- |
| macOS, Clang | Native suite passed 190/190 CTests; mock probe/plan/emit/strict round trip passed, including Stream generated dispatch and stale-route rejection; documented mock CMake example passed. |
| Raspberry Pi, GCC | The same expanded mock round trip passed. |
| Win11, MSVC | Generated mock Stream binding and dispatch ran; a strict Elide route against the Stream plan failed as stale. Existing native interrupt checks also passed. |
| Raspberry Pi, AVR GCC 14.2 | A named ATmega328P Stream queue instantiation compiled. The staged Uno Elide sketch compiled and its main-loop callback fired once in simavr. |
| Raspberry Pi, classic ESP32 / Arduino-ESP32 3.3.11 | The existing direct-interrupt sketch compiled and linked. This does not establish ESP32 deferred Stream support. |

The [Code P3](GrevirEventStreamCheckpoint-ReviewCode.md) notes that the AVR
Stream translation unit is not yet wired into a recurring target gate. The
[Surface P3](GrevirEventStreamCheckpoint-ReviewSurface-2.md) notes that linked
runnable examples still use Elide. Both are recorded follow-ups; neither
changes the supported semantics or the GO verdict. Stream carries no payload;
software-only events, named contexts, deadlines, ESP32 deferred dispatch, and
physical silicon validation remain outside this checkpoint.
