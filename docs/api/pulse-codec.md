# Pulse Codec API

**Package:** Grevir Pulse Codec. **Header:** `<GrevirPulseCodec.h>`.
**CMake target:** `grevir::pulse_codec`. **Dependencies:** Base and Time.

`setl::PweBitCollector<N>` stores a scalar payload of 1–32 bits; array
collectors store 1–65,535 bits in fixed `std::array` storage. The codec has no
GPIO, transport, dynamic-allocation or interrupt dependency. A waveform
definition such as `setl::PweWaveformParams1to3<Tick, BitPeriod, ...>` is held
by reference by `setl::PweEncoder<Bits>` and `setl::PweDecoder<Bits>`; keep it
alive and unchanged while they use it.

The clock uses unsigned integer ticks. Each bit interval and combined
stop/settle interval must be less than half the clock range, and calls must
occur often enough to make modular wraparound unambiguous. `send()` starts
when idle or queues one payload during the stop interval; it returns false
when that slot is occupied. Encoder `poll()` reports levels/deadlines, and the
caller must apply the requested transition on time. Decoder edge timestamps
must arrive in order. An unread completed value can be overwritten by a new
frame, setting the overrun flag. `reset()` clears partial data and flags.

The selected 8-bit composition has AVR compiler and simulator evidence;
broader collector widths have native tests only. See [Pulse IO](pulse-io.md)
for the GPIO module and [guide](../guides/pulse-io.md) for application use.
