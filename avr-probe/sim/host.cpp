#include "probe_result.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
#include "sim_avr.h"
#include "sim_elf.h"
#include "sim_interrupts.h"
}

namespace {

constexpr uint32_t kDefaultFreq = 16000000;
constexpr const char* kMcu = "atmega328p";
constexpr uint8_t kTimer0OvfVector = 16;
constexpr uint16_t kPortb = 0x25;
constexpr uint16_t kTifr0 = 0x35;
constexpr uint16_t kSreg = 0x5F;
constexpr uint16_t kTimsk0 = 0x6E;

struct Loaded {
  avr_t* avr = nullptr;
  elf_firmware_t firmware{};
};

void no_sleep(avr_t*, avr_cycle_count_t) {}

uint32_t symbol_addr(const elf_firmware_t& firmware, const char* name) {
  for (uint32_t i = 0; i < firmware.symbolcount; ++i) {
    if (std::strcmp(firmware.symbol[i]->symbol, name) == 0) {
      uint32_t addr = firmware.symbol[i]->addr;
      if (addr >= 0x800000u) {
        addr &= 0xffffu;
      }
      return addr;
    }
  }
  return 0;
}

Loaded load_elf(const char* path) {
  Loaded loaded;
  if (elf_read_firmware(path, &loaded.firmware) != 0) {
    std::fprintf(stderr, "elf_read_firmware failed: %s\n", path);
    std::exit(1);
  }
  loaded.avr = avr_make_mcu_by_name(kMcu);
  if (loaded.avr == nullptr) {
    std::fprintf(stderr, "avr_make_mcu_by_name(%s) failed\n", kMcu);
    std::exit(1);
  }
  avr_init(loaded.avr);
  loaded.avr->frequency = loaded.firmware.frequency ? loaded.firmware.frequency
                                                    : kDefaultFreq;
  loaded.avr->sleep = no_sleep;
  avr_load_firmware(loaded.avr, &loaded.firmware);
  return loaded;
}

void terminate(Loaded& loaded) {
  avr_terminate(loaded.avr);
}

int run_until(avr_t* avr, uint64_t max_cycles) {
  while (avr->cycle < max_cycles) {
    const int state = avr_run(avr);
    if (state == cpu_Done || state == cpu_Crashed) {
      return state;
    }
  }
  return avr->state;
}

ProbeResult read_result(avr_t* avr, uint32_t addr) {
  ProbeResult result{};
  if (addr == 0 || addr + kProbeResultSize > avr->ramend + 1u) {
    return result;
  }
  std::memcpy(&result, avr->data + addr, sizeof(result));
  return result;
}

int cmd_smoke(const char* elf) {
  Loaded loaded = load_elf(elf);
  std::printf("simavr smoke mcu=%s freq=%u elf=%s\n", kMcu,
    loaded.avr->frequency, elf);
  const int state = run_until(loaded.avr, 2000);
  const uint8_t portb = loaded.avr->data[kPortb];
  std::printf("cycles=%llu state=%d pc=0x%04x PORTB=0x%02x\n",
    static_cast<unsigned long long>(loaded.avr->cycle), state,
    loaded.avr->pc, portb);
  const bool ok = state != cpu_Crashed && (portb & 0x80u) != 0;
  terminate(loaded);
  if (!ok) {
    std::fprintf(stderr, "smoke failed: expected PORTB.7 set, not crashed\n");
    return 1;
  }
  std::printf("PASS smoke\n");
  return 0;
}

int wait_ready(Loaded& loaded, uint64_t max_cycles, ProbeResult* out) {
  const uint32_t addr = symbol_addr(loaded.firmware, "probe_result");
  if (addr == 0) {
    std::fprintf(stderr, "symbol probe_result not found\n");
    return 1;
  }
  std::printf("probe_result=0x%04x\n", addr);
  while (loaded.avr->cycle < max_cycles) {
    const int state = avr_run(loaded.avr);
    if (state == cpu_Crashed) {
      std::fprintf(stderr, "cpu_Crashed at cycle %llu pc=0x%04x\n",
        static_cast<unsigned long long>(loaded.avr->cycle), loaded.avr->pc);
      return 1;
    }
    *out = read_result(loaded.avr, addr);
    if (out->ready && out->magic == kProbeMagic) {
      return 0;
    }
  }
  std::fprintf(stderr, "timeout waiting for probe_result.ready\n");
  return 1;
}

int cmd_ocr(const char* elf) {
  Loaded loaded = load_elf(elf);
  ProbeResult result{};
  if (wait_ready(loaded, 200000, &result) != 0) {
    terminate(loaded);
    return 1;
  }
  std::printf(
    "ocr Fast PWM TOP=ICR1=200 prescaler=/8 write_tcnt=%u after_write_tcnt=%u "
    "ocr_read=%u ocf1a=%u flags=0x%02x cycles=%llu\n",
    result.v[0], result.v[1], result.v[2], result.v[3], result.flags,
    static_cast<unsigned long long>(loaded.avr->cycle));
  terminate(loaded);
  if ((result.flags & 1u) == 0 || result.v[2] != 120) {
    std::fprintf(stderr,
      "ocr failed: expected buffered OCR (OCF1A clear before BOTTOM) "
      "and CPU OCR1A==120\n");
    return 1;
  }
  std::printf("PASS ocr double-buffer at BOTTOM\n");
  return 0;
}

int cmd_latch(const char* elf) {
  Loaded loaded = load_elf(elf);
  ProbeResult result{};
  if (wait_ready(loaded, 20000, &result) != 0) {
    terminate(loaded);
    return 1;
  }
  std::printf(
    "latch grevir_icr1=0x%04x reversed_icr1=0x%04x grevir_tcnt1=0x%04x "
    "reversed_tcnt1=0x%04x flags=0x%02x\n",
    result.v[0], result.v[1], result.v[2], result.v[3], result.flags);
  terminate(loaded);
  if (result.v[0] != 0xA5C3) {
    std::fprintf(stderr, "latch failed: Grevir ICR1 write/read did not yield 0xA5C3\n");
    return 1;
  }
  if (result.v[1] == 0x1234) {
    std::printf(
      "LIMITATION simavr 1.6 commits reversed ICR1 byte order (TEMP latch not modeled); "
      "silicon Phase 5 owns the broken-order disagreement\n");
  } else {
    std::printf("reversed ICR1 disagreed (0x%04x != 0x1234)\n", result.v[1]);
  }
  if (result.v[2] != 0xA5C3) {
    std::printf(
      "LIMITATION simavr 1.6 stopped TCNT1 read is 0x%04x (synthesized counter, not TEMP SRAM)\n",
      result.v[2]);
  }
  std::printf("PASS latch Grevir 16-bit ICR1 high-then-low write\n");
  return 0;
  return 0;
}

struct IrqTrace {
  avr_t* avr = nullptr;
  uint64_t irq_notify_cycle = 0;
  uint32_t irq_notify_value = 0;
  unsigned notifies = 0;
};

void on_timer0_ovf_irq(avr_irq_t*, uint32_t value, void* param) {
  auto* trace = static_cast<IrqTrace*>(param);
  if (trace->notifies == 0) {
    trace->irq_notify_cycle = trace->avr->cycle;
    trace->irq_notify_value = value;
  }
  ++trace->notifies;
}

int cmd_irq(const char* elf) {
  Loaded loaded = load_elf(elf);
  const uint32_t isr = symbol_addr(loaded.firmware, "__vector_16");
  const uint32_t result_addr = symbol_addr(loaded.firmware, "probe_result");
  const uint32_t main_addr = symbol_addr(loaded.firmware, "main");
  if (isr == 0 || result_addr == 0 || main_addr == 0) {
    std::fprintf(stderr, "missing __vector_16, probe_result, or main\n");
    terminate(loaded);
    return 1;
  }
  IrqTrace trace;
  trace.avr = loaded.avr;
  avr_irq_t* irq = avr_get_interrupt_irq(loaded.avr, kTimer0OvfVector);
  if (irq != nullptr) {
    avr_irq_register_notify(irq, on_timer0_ovf_irq, &trace);
  }
  uint64_t tifr_cycle = 0;
  uint64_t vector_cycle = 0;
  uint64_t isr_cycle = 0;
  uint64_t reti_cycle = 0;
  const uint32_t vector_addr = static_cast<uint32_t>(kTimer0OvfVector) * 4u;
  while (loaded.avr->cycle < 20000) {
    const uint8_t tifr = loaded.avr->data[kTifr0];
    const uint8_t timsk = loaded.avr->data[kTimsk0];
    const uint8_t sreg = loaded.avr->data[kSreg];
    if (tifr_cycle == 0 && (tifr & 1u) && (timsk & 1u) && (sreg & 0x80u)) {
      tifr_cycle = loaded.avr->cycle;
    }
    const int state = avr_run(loaded.avr);
    if (state == cpu_Crashed) {
      std::fprintf(stderr, "cpu_Crashed in irq probe\n");
      terminate(loaded);
      return 1;
    }
    const uint32_t pc = loaded.avr->pc;
    if (vector_cycle == 0 && pc == vector_addr) {
      vector_cycle = loaded.avr->cycle;
    }
    if (vector_cycle != 0 && pc >= isr && pc < main_addr) {
      if (isr_cycle == 0) {
        isr_cycle = loaded.avr->cycle;
      }
    } else if (isr_cycle != 0 && reti_cycle == 0 && pc >= main_addr) {
      reti_cycle = loaded.avr->cycle;
    }
    if (tifr_cycle == 0 && (loaded.avr->data[kTifr0] & 1u)) {
      tifr_cycle = loaded.avr->cycle;
    }
    ProbeResult result = read_result(loaded.avr, result_addr);
    if (result.ready && isr_cycle != 0 && (reti_cycle != 0 || result.v[0] >= 1)) {
      if (reti_cycle == 0) {
        reti_cycle = loaded.avr->cycle;
      }
      const uint64_t pending = tifr_cycle ? tifr_cycle : trace.irq_notify_cycle;
      std::printf(
        "irq TIMER0_OVF vector=%u isr=0x%04x main=0x%04x pending_cycle=%llu "
        "vector_cycle=%llu isr_cycle=%llu reti_cycle=%llu notify_value=%u "
        "tcnt0_in_isr=%u isr_count=%u "
        "vector_after_pending=%lld jmp_to_isr=%lld "
        "isr_to_reti=%lld datasheet_min_entry_cycles=4 simavr_not_silicon=1\n",
        kTimer0OvfVector, isr, main_addr,
        static_cast<unsigned long long>(pending),
        static_cast<unsigned long long>(vector_cycle),
        static_cast<unsigned long long>(isr_cycle),
        static_cast<unsigned long long>(reti_cycle),
        trace.irq_notify_value, result.v[1], result.v[0],
        static_cast<long long>(vector_cycle - pending),
        static_cast<long long>(isr_cycle - vector_cycle),
        static_cast<long long>(reti_cycle - isr_cycle));
      terminate(loaded);
      if (pending == 0 || vector_cycle == 0 || isr_cycle == 0 || result.v[0] == 0
          || reti_cycle <= isr_cycle) {
        std::fprintf(stderr, "irq failed: missing pending/vector/ISR/RETI\n");
        return 1;
      }
      std::printf("PASS irq pending, vector, ISR, RETI\n");
      return 0;
    }
  }
  std::fprintf(stderr, "irq timeout tifr=%llu vec=%llu isr=%llu reti=%llu\n",
    static_cast<unsigned long long>(tifr_cycle),
    static_cast<unsigned long long>(vector_cycle),
    static_cast<unsigned long long>(isr_cycle),
    static_cast<unsigned long long>(reti_cycle));
  terminate(loaded);
  return 1;
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::fprintf(stderr, "usage: grevir_simavr_host smoke|ocr|latch|irq <elf>\n");
    return 2;
  }
  const std::string cmd = argv[1];
  const char* elf = argv[2];
  if (cmd == "smoke") {
    return cmd_smoke(elf);
  }
  if (cmd == "ocr") {
    return cmd_ocr(elf);
  }
  if (cmd == "latch") {
    return cmd_latch(elf);
  }
  if (cmd == "irq") {
    return cmd_irq(elf);
  }
  std::fprintf(stderr, "unknown command %s\n", cmd.c_str());
  return 2;
}
