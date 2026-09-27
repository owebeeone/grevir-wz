# Deferred event dispatch: strict-demand and capacity remediation

Status: implementation in review. This follow-up was authorized after the
[round-two NO-GO](GrevirEventDispatchCheckpoint-ReviewOutcome.md). It addresses
both P2 findings from that round and the three bounded documentation findings;
it does not expand the supported event contexts or MCU inventories.

| Finding | Correction | Closure evidence |
| --- | --- | --- |
| [Code P2-1](GrevirEventDispatchCheckpoint-ReviewCode-2.md) | Use one catalog-wide `DemandData` implementation in both passes. In the final generated translation unit, compare the live, post-header demand set with the emitted snapshot before accepting any binding. | A handler added only in strict compilation fails with `GREVIR_IRQ_STALE_DEMAND_SET` for zero-to-one and one-to-two cases; unchanged raw, direct and deferred plans still link. |
| [State P2-1](GrevirEventDispatchCheckpoint-ReviewState-2.md) | Check the board's original integral capacity against `1..2048` before converting it, only when deferred delivery is selected. Use that checked result for plan serialization and queue storage. Serialize capacity as a 16-bit schema field, and derive the queue index type from the selected count. | `0`, negative, `2049` and `65537UL` fail in the deferred probe; `1`, `2`, `255`, `256` and `2048` retain their selected values. Direct and zero-demand applications accept inactive capacity zero. AVR GCC rejects `65537UL` before emission. |
| [Surface S2-01–03](GrevirEventDispatchCheckpoint-ReviewSurface-2.md) | Align the ESP32 support statement; show qualified post outcomes; show overrun observation in the Uno sketch. | Read public pages together, compile mock and Uno examples, and build the named classic ESP32 direct example. |

The maximum is an admissible selected count, **not** an allocation. A board
selecting four records gets four event records. The present event queue keeps
its own locked, multi-producer publication, per-event elision and sticky
overrun semantics. Base's `CircularBuffer` has a capacity template and a
size-derived index type, but its write-after-overrun and producer contract
are different; this patch reuses `TypeForMaxValue` for the index rather than
substituting the entire buffer without a semantic review.

The schema and emitter identity advance together because the record's
capacity field changes from one to two bytes and strict output gains a live
demand assertion. The generator still lowers the C++-derived plan; it does
not rediscover handlers or choose capacity. Validation includes macOS native
tests, Raspberry Pi generated mock and target compiler builds, Win11/MSVC
generated mock and negative probes, AVR GCC plus simavr, and the classic ESP32
compile/link path. Physical silicon remains on hold.
