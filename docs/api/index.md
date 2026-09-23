# Package API index

Public entry headers and CMake targets are below. CMake package
configuration carries transitive dependencies; Arduino `library.properties`
declares them for installed libraries. All runtime packages require C++23.

| Package | Entry header | CMake target | Contract |
| --- | --- | --- | --- |
| Base | `GrevirBase.h` | `grevir::base` | [Base](base.md) |
| Time | `GrevirTime.h` | `grevir::time` | [Time](time.md) |
| Core | `GrevirCore.h` | `grevir::core` | [Core](core.md) |
| Peripherals | `GrevirPeripherals.h` | `grevir::peripherals` | [Peripherals](peripherals.md) |
| Registers | `GrevirRegisters.h` | `grevir::registers` | [Registers](registers.md) |
| AVR | `GrevirAVR.h` | `grevir::avr` | [AVR](avr.md) |
| Arduino | `GrevirArduino.h` | `grevir::arduino` | [Arduino](arduino.md) |
| Arduino AVR | `GrevirArduinoAVR.h` | `grevir::arduino_avr` | [Arduino AVR](arduino-avr.md) |
| Pulse Codec | `GrevirPulseCodec.h` | `grevir::pulse_codec` | [Pulse Codec](pulse-codec.md) |
| Pulse IO | `GrevirPulseIO.h` | `grevir::pulse_io` | [Pulse IO](pulse-io.md) |
| Packet | `GrevirPacket.h` | `grevir::packet` | [Packet](packet.md) |
| Encoder | `GrevirEncoder.h` | `grevir::encoder` | [Encoder](encoder.md) |
| Stepper | `GrevirStepper.h` | `grevir::stepper` | [Stepper](stepper.md) |
| FastLED | `GrevirFastLED.h` | `grevir::fastled` | [FastLED](fastled.md) |

Grevir Test Support is development-only, not a runtime dependency. A package's
presence in this index describes its API and host evidence; see
[support](../supported.md) for named target coverage.
