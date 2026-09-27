# Main-loop Stream delivery checkpoint

Status: implementation checkpoint awaiting review. This adds the already
described `MainLoop`/`Stream` delivery mode to the existing mock and ATmega328P
queue path. The event still comes from the finite module closure and activates
its hardware interrupt through the existing probe, allocator and target emitter.
The application selects the mode with `RouteFor<Event>` and declares one
`on_event<Event>()` handler; it supplies no second binding list.

Each accepted Stream firing occupies one queue record and invokes the handler
once when dispatched. A repeat never returns `coalesced`. If the selected
queue is full, the firing is dropped, `post` returns `full`, and sticky overrun
is set. The generated ISR bridge uses the same publication path. Elide and
Stream records share one FIFO; the queue lock serializes publication and
dequeue, while callbacks run outside it. A Stream record carries no payload
and no pending mark. `stop()` discards queued records, and `prepare()` resets
the queue and overrun state. Capacity still means the board-selected record
count, bounded by 2048, not a fixed allocation of 2048 records.

This checkpoint changes no probe schema or generator decision logic: handler
demand already records delivery mode, Python already validates `stream`, and
strict compilation already compares the live route with the generated plan.
The mock round trip now proves a Stream demand, generated entry, two firings
delivered, and rejection of a strict build using a stale Elide route. Native
queue tests cover repeat retention, full behavior, recovery, and stop/reset.
AVR GCC compiles a named Stream queue instantiation; the Uno simavr example
remains the Elide route, so simulated Stream timing is not claimed. Win11/MSVC
checks include the generated Stream route. ESP32 deferred synchronization,
named contexts, software-only events, payloads, and deadline scheduling remain
separate work. Physical silicon validation remains on hold.
