# Installed portable PWM MVP

The timer MVP now lives in the installed Core, Peripherals and AVR packages.
`experiments/timer-allocation` contains forwarding headers and verification fixtures;
it has no separate allocator or AVR implementation. The supported timer feature set
is unchanged: fixed-frequency synchronous fast PWM on ATmega328P Timer0/1/2.

## Application use

Include `<grevir/avr/devices/atmega328p/pwm_backend.hpp>` and link `grevir::avr`.
Its CMake package carries the Core, Peripherals, Registers and Base dependencies.
Supply the existing device byte-access and interrupt-barrier policies as `Device`:

```cpp
namespace p = grevir::pwm;
namespace avr_pwm = p::atmega328p;

using MotorPwm = p::Instance<"motor",p::PwmRequest<"pwm",
  p::Frequency<p::Hertz<1000>,p::WithinPpm<10'000>>,
  p::DutyStepAtMost<1,256>,
  p::For<p::Target::avr,p::Pin<avr_pwm::PB1>,
    p::Frequency<p::Hertz<1000>,p::Exact>,p::avr::TopFromIcr>,
  p::For<p::Target::esp32,p::Pin<18>,p::esp32::ApbClock>>>;

template <typename Allocation>
struct Motor : ardo::ModuleBase<
    ardo::Parameters<typename Allocation::template Pwm<"motor">>> {
  using Output = typename Allocation::template Pwm<"motor">;
  static void runSetup() { Output::write(1,4); }
};

using MotorModule = grevir::RequestedModule<setl::TypeArgs<MotorPwm>,Motor>;
using App = grevir::AllocatedApplication<
  avr_pwm::Backend<Device,16'000'000>,MotorModule>;
void setup_application() { App::runSetup(); }
void loop_application() { App::runLoop(); }
```

The common frequency interval is intersected with the resident AVR requirement;
ESP32 options remain inert. This example chooses Timer1 ICR TOP=15999 and quarter
duty OCR1A=3999. It does not create an ESP32 driver. The installed AVR consumer
compiles and executes this pattern using a native memory policy without Catch2 or
Test Support dependencies.

## Collection, ownership and lifecycle

`RequestedModule<Requests,ModuleTemplate,Claims,Dependencies...>` separates static
requirements from the module instantiated with selected drivers. Requests are a
`setl::TypeArgs` list of named `p::Instance` declarations. Optional Claims defaults
to `ardo::ResourceClaim<>`; dependencies are other module descriptors. Core collects
the descriptor dependency closure once. Request order does not affect allocation.
An equal nonzero sharing-group number on request instances requires one timer;
the default zero means an independent timer owner.

`grevir::ExistingModule<Module>` includes an ordinary Core module and its complete
dependency closure. Their parameter resource claims participate in allocation.
ATmega328P whole timer, timer-range and shared-use claims map to timer reservations;
GPIO claims use the same physical pad identities as the PWM backend. For example,
`ardo::HardwareTimer<1>` reserves Timer1 and `ardo::GPIOResource<avr_pwm::PB1>`
reserves PB1. These pad identities are not Arduino digital-pin numbers. Board pin
alias normalization is still future work. Optional explicit backend reservations
use the existing inventory resource IDs.

Declare other fixed claims on the descriptor (or through ExistingModule) so the
allocator can avoid them. Claims discovered only in a bound module's parameters
are still checked by Core and cannot silently overlap the allocation, but they
cause rejection rather than rerunning search. Automatic inference from arbitrary
module-template bodies is not implemented.

Each selected timer and its pins have one application owner. Endpoints carry empty
physical claims because that owner already holds them. Final Core validation checks
the owner against all bound modules and their dependencies. Exclusive resource
claims also conflict with shared-use claims; compatible shared users retain their
existing shared-use semantics when no exclusive owner is present.

`App::runSetup()` initializes each selected timer once before any parameter or
module setup callback, then runs the ordinary dependency-ordered lifecycle.
Descriptor dependencies become dependencies of the bound modules too.
`runLoop()` uses the existing module lifecycle and does not reconfigure timers.
A repeated explicit runSetup call performs initialization again.

## Boundaries and evidence

Core owns the generic bounded search and application assembly. Peripherals owns
PWM requirements, rational frequency windows, candidate validation and timer
compatibility. AVR owns declaration-derived candidates, resource mapping, setup
and duty writes. No target-specific code or dependency is introduced into Core.
The search budget and visit count are uint32_t, preserving 100,000 extensions on
16-bit-size_t targets; frequency template specializations use matching uint32_t
parameters rather than assuming unsigned is 32 bits. These are source-level
portability corrections, not target compiler evidence.

Duty and startup behavior are unchanged from the MVP: true GPIO zero/full endpoints,
interior OCR=high_ticks−1, downward fraction rounding, full-period writeTicks support,
caller-enabled clocks, synchronous Timer2, disabled timer interrupts and supplied
byte-access/barrier policies. No atomic or glitch-free live-update guarantee is made.
The inherited raw-OCR API remains a separate legacy interface.

Validation covers 125 native cases, the four allocation/oracle checks with ASan/UBSan,
a positive application compiler control plus six expected rejections, and isolated
production/install/consumer checks for all four affected packages. The application
checks cover shared-owner initialization once, setup before parameter callbacks,
dependency order, declaration reordering, existing dependency claims, whole/range/
shared timer reservations, GPIO reservations, duplicate identities and late claims.
AVR compiler and hardware validation remain on hold. The C++23 library/toolchain
requirements on an actual AVR have not been established by these native checks.

A real ESP32 backend, board/Arduino integration, target barrier implementation and
additional timer features remain separate work. Other AVR variants, asynchronous
operation, capture/interrupt APIs and portable runtime frequency changes stay TBD.
