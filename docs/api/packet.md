# Packet API

**Package:** Grevir Packet. **Header:** `<GrevirPacket.h>`.
**CMake target:** `grevir::packet`. **Dependency:** Base.

`ardo::FragmentSender<FragmentCount, PayloadSize>` fragments a byte message.
`ardo::FragmentReceiver<Slots, FragmentCount, PayloadSize, Address>` retains
a fixed number of reassembly slots. `Address` defaults to an opaque uint32
identity; a custom type must be default-constructible, copyable, assignable
and equality-comparable. Address, port and stream ID distinguish streams.
Neither type owns a network transport.

Fragment count is 1–32, payload capacity and slot count are positive, and
chosen capacities must fit uint32 lengths and the target's `size_t` address
space. Each receiver slot reserves `FragmentCount * PayloadSize` payload
bytes plus metadata; the sender uses one wire fragment on the stack.
Callbacks are synchronous. `send()` returning true means the callbacks ran,
not that a remote peer received data. Never retain a callback's pointer
without copying its bytes, reenter the same object from a callback, or access
it concurrently without external control.

The receiver accepts reordered fragments and ignores repeated copies after
the first accepted copy of an index. It uses empty, then completed, then
least-recently-progressed partial slots. Evicting a completed slot loses its
duplicate-delivery history. No timeout, acknowledgement or retransmission
policy is supplied. The stream ID includes a CRC-32C value, but payload CRC
is not verified during reassembly. See [guide](../guides/packet.md) and
[complete AVR example](../examples/packet-avr.md).
