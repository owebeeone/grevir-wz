#!/bin/sh
# Run on weftpi from this directory. Records ABI, header and dialect results.
set -eu

MCU=atmega328p
F_CPU=16000000UL
CXX="avr-g++ -mmcu=${MCU} -DF_CPU=${F_CPU} -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti"
OUT="${OUT:-./out}"
mkdir -p "${OUT}"
report="${OUT}/probe-report.txt"
: > "${report}"

log() {
  printf '%s\n' "$*" | tee -a "${report}"
}

compile_std() {
  std="$1"
  src="$2"
  elf="$3"
  log "COMPILE ${src} -std=${std}"
  if ${CXX} -std="${std}" -o "${elf}" "${src}"; then
    avr-size "${elf}" | tee -a "${report}"
    log "PASS ${src} ${std}"
    return 0
  fi
  log "FAIL ${src} ${std}"
  return 1
}

compile_header() {
  header="$1"
  std="$2"
  src="${OUT}/inc_$$.cpp"
  obj="${OUT}/inc_$$.o"
  printf '#include %s\nint main() { return 0; }\n' "${header}" > "${src}"
  if ${CXX} -std="${std}" -c -o "${obj}" "${src}"; then
    log "PASS include ${header}"
    rm -f "${src}" "${obj}"
    return 0
  fi
  log "FAIL include ${header}"
  rm -f "${src}" "${obj}"
  return 1
}

log "host=$(hostname)"
log "date=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
log "avr-g++=$(avr-g++ -dumpversion)"
avr-g++ -v 2>&1 | tail -1 | tee -a "${report}"
log "simavr=$(simavr -h 2>&1 | head -1 || true)"

std_ok=
for std in c++23 c++20 c++17; do
  if compile_std "${std}" abi.cpp "${OUT}/abi.elf"; then
    std_ok="${std}"
    break
  fi
done
if [ -z "${std_ok}" ]; then
  log "NO working -std= for abi.cpp"
  exit 1
fi
log "SELECTED_STD=${std_ok}"
compile_std "${std_ok}" features.cpp "${OUT}/features.elf" || true

log "---- headers ----"
for h in \
  '<limits.h>' '<stdint.h>' '<stddef.h>' '<assert.h>' '<stdlib.h>' '<string.h>' \
  '<avr/io.h>' '<avr/interrupt.h>' \
  '<climits>' '<cstdint>' '<cstddef>' '<cassert>' '<cstdlib>' '<cstring>' \
  '<array>' '<type_traits>' '<utility>' '<limits>' '<tuple>'
do
  compile_header "${h}" "${std_ok}" || true
done

log "---- cmake toolchain ----"
cmake -S . -B "${OUT}/cmake" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/../cmake/toolchains/avr-atmega328p.cmake"
cmake --build "${OUT}/cmake"
avr-size "${OUT}/cmake/grevir_avr_abi" "${OUT}/cmake/grevir_avr_features" | tee -a "${report}"
log "CMAKE_OK"
log "DONE"
