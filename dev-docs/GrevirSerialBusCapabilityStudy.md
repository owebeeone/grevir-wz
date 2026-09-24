# Serial-bus peripheral inventory and API questions

## State and scope

This is a first-pass **hardware inventory**, not a completed cross-MCU bus API
design or an implementation claim. The module-composition and resource-claim
mechanism already exists in Grevir Core; the bus-specific providers and
consumer bindings do not. The previous
[ESP32 capability study](GrevirEsp32CapabilityStudy.md) focused on timers/PWM,
GPIO and Parklights'
external SPI/RMT ownership. The [repository plan](GrevirRepositoryPlan.md)
proposes UART, I2C and SPI contracts;
[Core's resource graph](../grevir-core/src/grevir/core/resource_graph.hpp)
names those resource kinds. The current
[Arduino Serial wrapper](../grevir-arduino/src/grevir/arduino/serial.hpp) and
[ESP32 Serial0 adapter](../grevir-arduino-esp32/src/grevir/arduino_esp32/serial.hpp)
are narrow
application bindings, not a general UART backend. There is no Grevir I2C,
SPI or CAN/TWAI backend yet. Physical validation remains on hold.

The intended ESP32 Xtensa comparison is classic ESP32, ESP32-S2 and ESP32-S3
under Arduino-ESP32 3.3.11 / ESP-IDF 5.5.5. ESP32-C3 is included as a later
RISC-V contract check, not an implemented target. ESP8266 is a separate SDK
and toolchain study and is not represented by these ESP32 inventories. Counts
below are on-chip controllers, **not** available board resources.

| Target | I2C/TWI | General-purpose SPI | UART/USART | On-chip CAN controller |
| --- | ---: | ---: | ---: | --- |
| ATmega328P | 1 TWI | 1 SPI | 1 USART | None |
| ATmega328PB | 2 TWI | 2 SPI | 2 USART | None |
| Classic ESP32 | 2 I2C | 2 (SPI2, SPI3) | 3 UART | 1 TWAI |
| ESP32-S2 | 2 I2C | 2 (SPI2, SPI3) | 2 UART | 1 TWAI |
| ESP32-S3 | 2 I2C | 2 (SPI2, SPI3) | 3 UART | 1 TWAI |
| ESP32-C3 | 1 I2C | 1 (SPI2) | 2 UART | 1 TWAI |

The AVR counts come from the
[ATmega328P datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
and
[ATmega328PB datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/40001906C.pdf).
ESP32 counts come from the pinned SoC capability headers for
[classic ESP32](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32/include/soc/soc_caps.h),
[S2](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32s2/include/soc/soc_caps.h),
[S3](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32s3/include/soc/soc_caps.h) and
[C3](https://github.com/espressif/esp-idf/blob/v5.5.5/components/soc/esp32c3/include/soc/soc_caps.h).
SPI1 is associated with external memory and is not counted as a normal
application SPI bus; see the
[ESP32 SPI documentation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/spi_master.html).

The AVR USART can also run as an SPI-compatible **master**. This is an
alternative use of the same USART, not an extra independent SPI controller;
claiming it for SPI must exclude simultaneous UART use.
[ATmega328P USART-in-SPI mode](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)

ESP32 TWAI implements classical CAN frames on these four chips, not CAN FD,
and needs an external transceiver for a physical CAN bus. An ATmega328P/PB
application would need an external CAN controller, usually consuming SPI,
chip-select, interrupt and transceiver resources; it is not an AVR CAN backend.
[ESP32 TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/peripherals/twai.html),
[S2 TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s2/api-reference/peripherals/twai.html),
[S3 TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/peripherals/twai.html),
[C3 TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/peripherals/twai.html)

## Dependent modules provide the sharing model

The intended pattern is a **provider module** for a physical bus controller.
It owns initialization, common pins/configuration and access serialization,
then exposes resource identities and operations to consumers. Each consumer
declares that provider in `DependentModules` and claims the particular resource
the provider makes available. The application adds the provider once even when
several consumers depend on it, starts it before them, and checks claims across
the complete dependency closure. Sharing the provider also shares its setup
code and state. This is the existing Ardoinus/Grevir composition model, not a
new bus-specific ownership scheme.
[Module dependency closure](../grevir-core/src/grevir/core/module.hpp),
[application lifecycle](../grevir-core/src/grevir/core/application.hpp),
[resource claims](../grevir-core/src/grevir/core/resource_claims.hpp)

The provider's whole-controller claim and its consumers' subresource claims
must be **different claim identities**: a consumer does not claim the physical
controller a second time merely to use a device on it. The existing
`shared_use_claim` can represent compatible use of a common setting; identical
settings coexist and incompatible ones conflict. A bus binding still needs to
specify what is fixed at setup and what can change per transaction. For example,
two SPI devices may use different clock rates and modes on one controller if
the provider switches those settings for each serialized transaction.
Generic `ResourceClaim` detects collisions among declared types, but by itself
does not prove that a claimed device/endpoint was actually offered by the
specified provider. The bus-specific binding must establish that relationship.

The portable layer should describe each provider's required behavior and a
stable request identity; chip and board profiles enumerate realizable
controllers, signal routes, DMA/interrupt options and external reservations.
Reordering declarations must not change assignments. These are requirements
for future bus providers, not claims that they are implemented today.

| Bus | Provider module owns | Consumer claims | Remaining bus-specific contract questions |
| --- | --- | --- | --- |
| I2C/TWI | Controller, SDA/SCL route, electrical setup and transaction sequencing | Addressed device or target endpoint | Controller versus target mode, clock stretching, repeated start, ten-bit addressing, transaction atomicity, timeout/recovery, address collision and multimaster behavior. |
| SPI | Controller, shared signals, setup and transaction serialization | Device chip-select and required transaction capabilities | When/how mode and rate change per device; AVR USART-as-SPI excludes UART use; ESP flash SPI is reserved; DMA, line widths and slave mode are capabilities. |
| UART/USART | Controller, clock source, routed signals and buffering policy | A stream endpoint or configured client role | Baud error tolerance, framing/parity, asynchronous I/O, flow control, and whether console/USB CDC already owns the endpoint. USB serial is not necessarily a UART. |
| CAN/TWAI | Controller, bit timing, transceiver pins, filters and bus state | Frame subscriptions and transmit clients | Classical CAN versus FD, bus-off recovery, multi-client filter composition, queues/callbacks and error reporting. AVR external-controller modules should provide the same higher-level service without pretending the controller is on-chip. |

For the next design pass, implement each bus as a provider/consumer use of the
existing dependency and claim machinery. Specify its transaction/result
semantics, offered subresources, compatibility rules and runtime serialization.
Then choose the backend integration appropriate to that bus; bypassing LEDC
software APIs does not decide how SPI, I2C, UART or TWAI should be driven.
Parklights' Arduino SPI display library needs a provider adapter or an explicit
external reservation so the declarative graph represents its real use.
Validate provider candidates against both 328P and 328PB so a
single-controller AVR assumption does not enter the common API.
