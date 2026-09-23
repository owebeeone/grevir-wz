#!/bin/sh
# Native libsimavr driver on weftpi. AVR ELFs come from build/avr-atmega328p.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)"
HOST="${ROOT}/build/simavr-host/grevir_simavr_host"
AVR="${ROOT}/build/avr-atmega328p/avr-probe"
ARDUINO_AVR="${ROOT}/avr-probe/arduino-cli/out/pulse-io-loopback/AvrLoopback.ino.elf"
"${HOST}" smoke "${AVR}/grevir_avr_firmware"
"${HOST}" ocr "${AVR}/grevir_avr_sim_ocr"
"${HOST}" latch "${AVR}/grevir_avr_sim_latch"
"${HOST}" irq "${AVR}/grevir_avr_sim_irq"
"${HOST}" pulse-io "${ARDUINO_AVR}"
