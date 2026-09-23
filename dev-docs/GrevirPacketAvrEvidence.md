# Packet ATmega328P validation — 23 September 2026

On weftpi (`gianni@10.1.1.236`), workspace root commit
`bab54d206ece83c67b7e41ac634a20b3fc53c843`, Packet member
`8e6779e9482f74f21664022a52c3d330c030279e` and Base member
`1dee17c591f9fc8447b568827c4aed8597a0379b` were used for the target
checks. The compiler was Debian `avr-g++` 14.2.0 in C++23 mode, with
Arduino CLI 1.5.2-rc.1 and Arduino AVR 1.8.8. The target has no installed
AVR libstdc++; Packet uses Grevir Base compatibility headers and C string
functions. No physical board was connected.

`PATH=/home/gianni/.local/bin:$PATH sh avr-probe/arduino-cli/run.sh` compiled
the checked-in `grevir-packet/examples/AvrPacket` sketch for Uno and passed
the other positive sketches and the expected Timer0 resource rejection. The
Packet sketch uses `FragmentSender<2, 8>` and `FragmentReceiver<1, 2, 8>` to
loop back a nine-byte payload through two frames. Arduino CLI reported
**1,896 program bytes and 85 RAM bytes**. The Uno ELF at
`avr-probe/arduino-cli/out/packet-avr/AvrPacket.ino.elf` has SHA-256
`3e7678b2a83f4bcf8429062c33f2a61f6783f6ea2b735ffc3cd60223737531af`.

The same sketch compiled for `arduino:avr:nano` with the Base and Packet
libraries, Debian compiler path and `-std=c++23`. Arduino CLI again reported
**1,896 program bytes and 85 RAM bytes**; the Nano ELF SHA-256 is
`9520baf5d9266c55f7af5876bbed24a5ad686a41289182fdcc5966d22dae8f32`.
These are compile and link results; the sketch's `packet_status` was not read
on a physical board.

The separate `grevir_avr_packet_loopback` firmware target built with CMake
and passed `sh avr-probe/sim/run.sh` under simavr 1.6 at 16 MHz. The host
checked the firmware's exported result, frame count and delivery count:
**two frames, one correct delivery, 6,223 simulated cycles**. `avr-size`
reported **1,782 program bytes and 65 data bytes** for this smaller standalone
firmware. Its ELF at `build/avr-atmega328p/avr-probe/grevir_avr_packet_loopback`
has SHA-256
`2d5639da61504fde89c5bef4f6cd9b3a4fe172c89b13068ca336d55ed618b207`.

This evidence covers one bounded sender/receiver composition on ATmega328P.
The sketch sizes and firmware size describe different programs. The simulator
run establishes that composition's behavior in simavr, not silicon timing,
all Packet template capacities, a network transport, or physical delivery.
Hardware validation remains on hold.
