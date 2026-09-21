// Compile-time ABI probe for avr-g++ / ATmega328P. Uses avr-libc C headers
// because Debian gcc-avr 14.2 does not install libstdc++.

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static_assert(CHAR_BIT == 8, "CHAR_BIT");
static_assert(sizeof(char) == 1, "sizeof char");
static_assert(sizeof(int) == 2, "AVR int is 16-bit");
static_assert(sizeof(long) == 4, "AVR long is 32-bit");
static_assert(sizeof(long long) == 8, "sizeof long long");
static_assert(sizeof(void*) == 2, "AVR data pointer");
static_assert(sizeof(size_t) == 2, "sizeof size_t");
static_assert(sizeof(ptrdiff_t) == 2, "sizeof ptrdiff_t");
static_assert(sizeof(uint8_t) == 1, "uint8_t");
static_assert(sizeof(uint16_t) == 2, "uint16_t");
static_assert(sizeof(uint32_t) == 4, "uint32_t");
static_assert(static_cast<int>(-1) < 0, "signed wrap polarity");
static_assert(static_cast<unsigned int>(-1) > 0u, "unsigned wrap polarity");

int main() {
  return static_cast<int>(sizeof(int));
}
