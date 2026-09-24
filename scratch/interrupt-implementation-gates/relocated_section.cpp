#if defined(__APPLE__)
#define GREVIR_PROBE_SECTION __attribute__((used, section("__DATA,__grevir_irq")))
#else
#define GREVIR_PROBE_SECTION __attribute__((used, section(".grevir_irq_plan")))
#endif

extern "C" int other_object;
extern "C" GREVIR_PROBE_SECTION const void* const grevir_irq_plan = &other_object;
