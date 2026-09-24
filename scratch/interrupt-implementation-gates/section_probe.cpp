// Small, target-compiled metadata record with no pointers or relocations.
#if defined(__APPLE__)
#define GREVIR_PROBE_SECTION __attribute__((used, section("__DATA,__grevir_irq")))
#else
#define GREVIR_PROBE_SECTION __attribute__((used, section(".grevir_irq_plan")))
#endif

struct Record {
  unsigned char bytes[24];
};

consteval Record make_record() {
  Record result{{'G', 'I', 'R', 'Q', 1, 0, 24, 0,
                 'm', 'o', 'c', 'k', 0, 0, 0, 0,
                 'm', 'o', 't', 'o', 'r', '.', '1', 0}};
  return result;
}

extern "C" GREVIR_PROBE_SECTION constinit const Record grevir_irq_plan = make_record();
