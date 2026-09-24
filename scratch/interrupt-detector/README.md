# Interrupt generation scratch probe

This is a host C++23 proof of interrupt discovery, allocation handoff, and ISR
source generation. The Python script generates an ATmega328P ISR source file and
compiles its non-AVR mock branch. It is not an installed Grevir API or an AVR
target validation result.

Run `python3 -B check_binding_set.py` to verify that the generated binding set
accepts the discovered handler and rejects both a missing handler binding and
a binding for an event without a handler. No independent interrupt resource
claim participates in this check. The selected plan remains a hand-written
fixture, and this prototype does not yet validate a peripheral-owner identity
or support an ESP32 emitter.

Run `python3 -B check_key_gate.py` for a separate host syntax probe of the
revised strict gate. It accepts a mapped event, rejects an unbound catalog
event, and rejects a foreign event that copies a valid event's stable key. The
stand-in generated declaration contains only the stable key, not a C++ event
type spelling.

Run from this directory:

```sh
python3 generate.py prepare --catalog event_catalog.json \
  --output /tmp/grevir-irq-roundtrip/discovery.cpp
c++ -std=c++23 -O2 -I. -c /tmp/grevir-irq-roundtrip/discovery.cpp \
  -o /tmp/grevir-irq-roundtrip/discovery.o
nm -u /tmp/grevir-irq-roundtrip/discovery.o | c++filt
python3 generate.py inspect --catalog event_catalog.json \
  --object /tmp/grevir-irq-roundtrip/discovery.o \
  --output /tmp/grevir-irq-roundtrip/discovered_handlers.json
python3 generate.py emit --catalog event_catalog.json \
  --manifest /tmp/grevir-irq-roundtrip/discovered_handlers.json \
  --plan selected_plan.json --out-dir /tmp/grevir-irq-roundtrip
c++ -std=c++23 -O2 -DGREVIR_IRQ_HOST_MOCK=1 -I. -I/tmp/grevir-irq-roundtrip \
  strict_main.cpp /tmp/grevir-irq-roundtrip/generated_isrs.cpp \
  -o /tmp/grevir-irq-roundtrip/strict-app
/tmp/grevir-irq-roundtrip/strict-app
```

`event_catalog.json` lists two logical events, without allocating a physical
vector. The discovery translation unit probes both. On Apple Clang 21 its
object has these undefined probes:

```text
void grevir_irq_probe<1001u, true>()
void grevir_irq_probe<1002u, false>()
```

It also contains an undefined C symbol with the catalog's SHA-256 fingerprint.
Python requires that fingerprint and exactly one probe per catalog event, then
writes a manifest listing event 1001 as handled. **Only after discovery** may
the allocator select physical bindings. `selected_plan.json` is a hand-written
stand-in for that allocator output. It binds event 1001 to Timer1 overflow and
contains no binding for unhandled event 1002. Python checks that the plan's
event set equals the discovered handler set, that each event type matches the
catalog, and that bindings, physical sources and vectors have unique owners.
This deliberately rejects two registrations for the same hardware source. The prototype
allows only `TIMER1_OVF_vect` in its device map.

The emitted detector header specializes `DetectorImpl` for event 1001. The
emitted source contains `ISR(TIMER1_OVF_vect)` under `__AVR__` and a host mock
entry only when `GREVIR_IRQ_HOST_MOCK` is explicitly defined. Unsupported
targets fail preprocessing. The strict host build calls the mock entry and observes the
user's handler. The discovery object is never linked into firmware. The event
catalog is sorted by stable ID before hashing and emission, so reordering its
records does not change the result.

The separate `unmapped.cpp` proves the strict compile-time gate:

```sh
c++ -std=c++23 -fsyntax-only unmapped.cpp
```

That command must fail at the handler specialization because the detector is
incomplete. `mapped.cpp` and `generated_detector.hpp` are older stand-ins for
the strict side. The default template names now represent real concepts:
`BindingId` is the selected binding's identity and
`DetectorImpl<Event>::binding_id` is generated only for handled events.

## Holes exposed by the round trip

- The catalog and selected plan are hand-written fixtures. Production must
  derive the catalog from the module closure, feed the discovered handler set
  into Grevir's allocator, and serialize the allocator's selected bindings.
  Event IDs must be stable, unique and independent of declaration order.
  Without that bridge, Python cannot know which timer/vector is legal.
- Handler specializations must be visible during discovery and strict
  compilation. Event types must be declared before the generated detector
  header, which must precede the handler header. Ordinary sketch layout and
  handlers defined only in a separate translation unit need an integration
  contract. C++ cannot enumerate arbitrary unseen specializations. A visible
  declaration without a definition passes discovery but must fail the strict
  link if the ISR calls it.
- The symbol reader currently supports Apple/GNU-style `nm` plus `c++filt` and
  the demangled spelling of integer/bool template arguments. Other object
  formats and compilers need adapters or a structured manifest. Qualified
  type names without template arguments are the only accepted `cpp_type`
  values here; a production catalog needs a general naming or alias scheme.
- A build must regenerate from a current discovery object and treat failure
  in preparation, compilation, inspection, allocation or generation as fatal.
  The catalog fingerprint catches a stale object after a catalog change;
  the strict compile catches added or removed handlers. A robust build driver
  still needs to prevent stale generated files from being reused after failure.
- The generated AVR entry is only a function definition. The selected plan
  must also configure and enable the peripheral source, define flag clearing
  and ordering, and account for interrupt latency and handler constraints.
  The script cannot prove that Arduino core or another library owns no vector.
  Shared-vector dispatch is deliberately unsupported. The emitted AVR branch
  has not been compiled or run on AVR; silicon hardware validation is held.

The earlier `discovery.cpp` is a minimal type-name marker experiment. The
Python-generated discovery translation unit replaces it for this cycle.
