#!/bin/sh
# Compile Grevir Arduino sketches on weftpi. Do not run on the Mac.
set -eu
ROOT="${GREVIR_ROOT:-$HOME/git/grevir-wz}"
FQBN="${FQBN:-arduino:avr:uno}"
CLI="${ARDUINO_CLI:-arduino-cli}"
BUILD_DIR="${BUILD_DIR:-$ROOT/avr-probe/arduino-cli/out}"
mkdir -p "$BUILD_DIR"

LIBS=""
for lib in grevir-base grevir-time grevir-core grevir-peripherals grevir-registers grevir-avr grevir-arduino grevir-arduino-avr grevir-pulse-codec grevir-pulse-io grevir-packet; do
  LIBS="$LIBS --library $ROOT/$lib"
done

EXTRA="${COMPILER_CPP_EXTRA_FLAGS:--std=c++23}"
PATH_OVERRIDE="${COMPILER_PATH:-/usr/bin/}"

compile_one() {
  sketch="$1"
  name="$2"
  echo "==== compile $name ===="
  "$CLI" compile --fqbn "$FQBN" $LIBS \
    --build-path "$BUILD_DIR/$name" \
    --build-property "compiler.path=$PATH_OVERRIDE" \
    --build-property "compiler.cpp.extra_flags=$EXTRA" \
    "$sketch"
}

compile_one "$ROOT/grevir-arduino/examples/BareMinimum/BareMinimum.ino" bareminimum
compile_one "$ROOT/grevir-arduino/examples/Blink/Blink.ino" blink
compile_one "$ROOT/grevir-arduino/examples/SerialHello/SerialHello.ino" serialhello
compile_one "$ROOT/grevir-pulse-io/examples/AvrLoopback/AvrLoopback.ino" pulse-io-loopback
compile_one "$ROOT/grevir-packet/examples/AvrPacket/AvrPacket.ino" packet-avr

echo "==== reserved Timer0 must fail ===="
set +e
compile_one "$ROOT/grevir-arduino/examples/ReservedTimerFail/ReservedTimerFail.ino" reservedfail
status=$?
set -e
if [ "$status" -eq 0 ]; then
  echo "ERROR: ReservedTimerFail compiled; expected Timer0 conflict"
  exit 1
fi
echo "ReservedTimerFail rejected as expected"
echo "Arduino CLI sketches: BareMinimum, Blink, SerialHello, AvrLoopback, AvrPacket compiled; reserved Timer0 rejected"
