# Grevir documentation

This is the public entry point for Grevir. Read
[supported platforms and evidence](supported.md) before assuming a backend or
board is available. The API uses C++23 and retains several `ardo`, `setl`,
`quad` and `step` names. These `/docs` pages are the current API contract;
package READMEs and `dev-docs` retain development checkpoint history.

| If you want to... | Read |
| --- | --- |
| Install libraries or build a native consumer | [Installation](install.md) |
| Understand modules, dependencies and lifecycle | [Modules](concepts/modules.md) |
| Understand resource claims and conflicts | [Resources](concepts/resources.md) |
| Bind GPIO and clocks without hard-coding an MCU | [Pins and clocks](concepts/pins-and-clocks.md) |
| Blink an Uno/Nano LED | [Blink example](examples/blink.md) |
| Request PWM with common and target-specific requirements | [PWM guide](guides/pwm.md) and [complete host example](examples/pwm-host.md) |
| Bind a timer interrupt handler | [Interrupt guide](guides/interrupts.md), [native mock example](examples/interrupt-mock/CMakeLists.txt), [Uno example](examples/interrupt-avr/interrupt-avr.ino), and [classic ESP32 example](examples/interrupt-esp32/interrupt-esp32.ino) |
| Transmit and receive pulse-encoded values | [Pulse IO guide](guides/pulse-io.md) and [AVR example](examples/pulse-io-avr.md) |
| Fragment and reassemble packets | [Packet guide](guides/packet.md) and [AVR example](examples/packet-avr.md) |
| Read a quadrature encoder or drive a stepper | [Motion guide](guides/motion.md), [encoder host example](examples/encoder-host.md), [stepper host example](examples/stepper-host.md) |
| Control a FastLED strip on Uno | [FastLED API](api/fastled.md) and [AVR sketch](examples/fastled-avr.md) |
| Find a package's public contract | [API package index](api/index.md) |

Every example states its target and dependencies. An example described as
"host tested" is not a claim that it ran on an MCU; an "AVR compiled" example
was built for the target but may not have run on a physical board. These terms
are defined in [supported platforms and evidence](supported.md).
