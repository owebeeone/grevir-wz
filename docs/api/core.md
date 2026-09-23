# Core API

**Package:** Grevir Core. **Header:** `<GrevirCore.h>` or
`<grevir/core/...hpp>`. **CMake target:** `grevir::core`.
**Dependencies:** Base.

`ardo::ModuleBase<Params, Deps>` declares a module's parameters and dependent
modules; both arguments have empty defaults. `ardo::Parameters<P...>` carries
parameter types and exposes zero-based `Param<N>`. `ardo::Application<M...>`
builds the dependency closure, checks resource claims and exposes static
`runSetup()` and `runLoop()` methods. Dependency cycles, duplicate exclusive
resource claims and conflicting ranges fail at compile time. See
[Modules](../concepts/modules.md) and [Resources](../concepts/resources.md).

`grevir::RequestedModule` and `grevir::AllocatedApplication` form the installed
portable PWM application path. Requests carry stable identities; a backend
provides candidates; the allocated binding reaches the module as a type.
Declaration order does not change an allocation for the same requests and
inventory. [PWM](../guides/pwm.md) describes the supported ATmega328P scope.

The older `SelectionResolver` remains a pass-through placeholder. New
applications should use the installed allocation path for its supported PWM
scope. Core's default sequential lifecycle does not imply interrupt safety or
ownership of physical device registers.
