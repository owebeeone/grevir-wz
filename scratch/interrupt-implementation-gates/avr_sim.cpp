#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
#include <sim_avr.h>
#include <sim_elf.h>
}

int main(int argc, char** argv) {
  if (argc != 2) { return 2; }
  elf_firmware_t firmware{};
  if (elf_read_firmware(argv[1], &firmware) != 0) { return 3; }
  auto* avr = avr_make_mcu_by_name("atmega328p");
  if (avr == nullptr) { return 4; }
  avr_init(avr);
  avr->frequency = 16000000;
  avr_load_firmware(avr, &firmware);
  std::uint32_t ticks_address = 0;
  for (std::uint32_t i = 0; i < firmware.symbolcount; ++i) {
    if (std::strcmp(firmware.symbol[i]->symbol, "grevir_irq_test_ticks") == 0) {
      ticks_address = firmware.symbol[i]->addr & 0xffffu;
    }
  }
  if (ticks_address == 0 || ticks_address > avr->ramend) {
    avr_terminate(avr);
    return 5;
  }
  while (avr->cycle < 200000) {
    const int state = avr_run(avr);
    if (state == cpu_Crashed) {
      avr_terminate(avr);
      return 6;
    }
    if (avr->data[ticks_address] != 0) {
      const auto ticks = avr->data[ticks_address];
      std::printf("PASS AVR Timer1 overflow handler: ticks=%u cycles=%llu\n",
        ticks, static_cast<unsigned long long>(avr->cycle));
      avr_terminate(avr);
      return ticks == 1 ? 0 : 7;
    }
  }
  std::fprintf(stderr, "Timer1 overflow handler did not run\n");
  avr_terminate(avr);
  return 8;
}
