# Pulse IO ATmega328P validation — 23 September 2026

This checkpoint was run on weftpi (`gianni@10.1.1.236`) at workspace
`/home/gianni/git/grevir-wz`, source root commit
`aebef368fe656ef1d699add673674010efdbdc4c`. The Pulse IO member was at
`2c62ebda752de7b3a442194b7d347f6ed646dbbb` and Pulse Codec at
`6cccacee2dfdc6e8b03d12ef6cd27fbcb653f5fc`.

The selected `grevir-pulse-io/examples/AvrLoopback` composition compiled and
linked for `arduino:avr:uno` with Arduino CLI 1.5.2-rc.1, Arduino AVR 1.8.8,
Debian `avr-g++` 14.2.0 (`compiler.path=/usr/bin/`) and `-std=c++23`. It uses
Grevir's Arduino AVR pin adapter, `MicrosClock`, Pulse Codec, and Pulse IO Core
modules. The linked ELF uses **2,854 flash bytes and 86 RAM bytes**. Its SHA-256
is `3676092a0d6982703b3ffee517086a65c3ca68425ef3aae448b0cb28a5b38e5e`;
the retained artifact is
`avr-probe/arduino-cli/out/pulse-io-loopback/AvrLoopback.ino.elf` on weftpi.
The standalone Pulse IO header-composition preflight also passed with
`-mmcu=atmega328p -Os -fno-exceptions -fno-rtti` and Grevir dependency includes,
with no AVR libstdc++.

`sh avr-probe/arduino-cli/run.sh` passed BareMinimum, Blink, SerialHello and
AvrLoopback and rejected ReservedTimerFail with the expected Core resource
conflict. `sh avr-probe/sim/run.sh` passed its prior probes and the new Pulse IO
probe under simavr 1.6 at 16 MHz. The simulator connects PD5/D5 output to
PD4/D4 input using libsimavr pin IRQs. It received the second frame as byte `1`
after **33 output changes and 30,281 simulated cycles**. The sketch configures
D4 as an externally driven input without the AVR pull-up.

This demonstrates compilation, linkage, resource composition and one simulated
8-bit loopback path. It does not measure real pin timing or validate physical
loopback, noise tolerance, other boards, all Pulse Codec collector widths, or
silicon behavior. Hardware validation remains on hold.
