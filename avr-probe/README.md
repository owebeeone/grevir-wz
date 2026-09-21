# AVR compiler probe

Tiny ATmega328P programs used on weftpi (`gianni@10.1.1.236`) to record
`avr-g++` ABI, language mode and header availability. They are not Grevir
firmware.

From this directory on weftpi:

```sh
./run.sh
```

Uses [../cmake/toolchains/avr-atmega328p.cmake](../cmake/toolchains/avr-atmega328p.cmake).
Phase 1 results land in `out/probe-report.txt`. The workspace AVR tree is
`build/avr-atmega328p`: public-header probes, `grevir_avr_firmware`, and one
Core claim-collision expected failure (case 2) with a passing control (case 1).
Debian gcc-avr 14.2 compiles C++23 but does not install libstdc++; Grevir AVR
builds leave `HAS_STD_LIB` unset and use Base compat fallbacks.

simavr 1.6 probes (weftpi native `build/simavr-host`):

```sh
cmake -S avr-probe/sim -B build/simavr-host -G Ninja
cmake --build build/simavr-host
./avr-probe/sim/run.sh
```

That runs smoke, Timer1 Fast PWM OCR double-buffer at BOTTOM, ICR1 16-bit access,
and TIMER0_OVF pending/vector/ISR/RETI. Do not configure `avr-probe/sim` with the
AVR toolchain.
