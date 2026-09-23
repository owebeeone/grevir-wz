# Fragment and reassemble packets

Grevir Packet divides a message into bounded wire fragments and reassembles
them by peer address, port and stream ID. It owns no UDP, Wi-Fi or other
transport. The application sends each produced wire packet through its own
transport and passes received bytes to the receiver.

```cpp
#include <GrevirPacket.h>

ardo::FragmentSender<2, 8> sender;
ardo::FragmentReceiver<1, 2, 8> receiver;
```

Here the sender permits at most two fragments of eight payload bytes; the
receiver retains one fixed slot. The sender uses one wire-fragment buffer on
the stack. Each receiver slot holds `fragment_count * fragment_size` payload
storage plus metadata. Choose capacities against the target's RAM and `size_t`
range. Fragment count is 1–32, and fragment payload capacity must be positive.

`send(data, length, callback)` calls the callback synchronously for each wire
packet. It returns false for an oversized message or a null pointer with
nonzero length. Success means callbacks ran; it does not mean a network delivered
the packets. Receive callbacks are also synchronous. Copy bytes that must
survive a callback, and do not reenter the same sender or receiver from a
callback. External synchronization is required for concurrent access.

The 14-byte framing prefix uses marker `0f 72 aa 47`, an eight-byte stream ID,
fragment index and last-fragment index. Unmarked short packets pass through;
a short payload starting with the reserved marker is wrapped to avoid ambiguous
framing. The first accepted copy of a fragment wins. Reordering and duplicates
are supported within the bounded slot pool. A completed ID suppresses repeat
delivery only while its slot remains retained. There is no timeout,
acknowledgement or retransmission policy. The stream ID contains CRC-32C, but
the receiver does **not** verify payload CRC on reassembly; do not rely on it
for corruption detection.

The [complete Uno/Nano example](../examples/packet-avr.md) loops the sender
directly into the receiver. That selected two-fragment composition compiled for
both boards and passed in simavr. See the [Packet API contract](../api/packet.md)
for address type and buffer-selection rules.
