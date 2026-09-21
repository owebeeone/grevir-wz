#pragma once

#include <stdint.h>

// Shared layout for AVR firmware SRAM and the weftpi libsimavr host.
// Packed so AVR and aarch64 agree. Magic is little-endian 'GRV1'.
struct ProbeResult {
  uint32_t magic;
  uint16_t v[4];
  uint8_t flags;
  uint8_t ready;
} __attribute__((packed));

static const uint32_t kProbeMagic = 0x31565247u;
static const uint32_t kProbeResultSize = 14u;
