# Modules and application lifecycle

Grevir represents an application as a compile-time set of module types. A
module can declare parameters and dependencies through `ardo::ModuleBase`.
`ardo::Application<Modules...>` closes the dependency graph, checks resource
claims and runs setup and loop callbacks. On Uno/Nano, use
`ardo::ArduinoAvrApplication<Modules...>` to add the Arduino Timer0 reservation.

```cpp
#include <GrevirCore.h>

struct Sensor : ardo::ModuleBase<> {
  static void runSetup() {}
  static void runLoop() {}
};

using App = ardo::Application<Sensor>;
// Call App::runSetup() once, then App::runLoop() repeatedly.
```

`App::runSetup()` runs parameter setup callbacks before module setup callbacks;
`App::runLoop()` similarly runs parameter loop callbacks before module loop
callbacks. Dependencies run before their consumers within each phase, and a
shared dependency module is visited once. Dependency cycles fail compilation.
The application and module interfaces are static; they do not create a runtime
registry or allocate a module object per declaration. A module may itself hold
static state, as in the [Blink example](../examples/blink.md).

`ardo::Parameters<P...>::Param<N>` selects the zero-based parameter type.
An empty or out-of-range index is a compile-time error. `ardo::DependentModules`
declares dependencies. Resource claims carried by parameters and modules are
checked during application construction; see [Resources](resources.md).

This lifecycle is sequential. A module using callbacks, interrupts or shared
state must provide any required synchronization through its backend or its own
policy. See the [Core API contract](../api/core.md) and [Arduino AVR API](../api/arduino-avr.md).
