# Grevir — first 30 files in each proposed repository

Query results from [GrevirFileMap.sqlite](GrevirFileMap.sqlite), using
[GrevirFirst30Files.sql](GrevirFirst30Files.sql).

Each section shows up to **30 distinct destination files**, sorted alphabetically
by their planned path inside that repo. Contributing original source files are
grouped alongside each destination. Original paths are relative to `ardoinus/`.
Detailed API notes and hashes remain in SQLite.

The database holds **736 original source files, 796 mapping entries and 19 proposed
repositories**. This view covers the existing C++/sketch file map, including
generated data, tests, examples and retained legacy files. Additional new APIs and
non-C++ tooling/platform files remain described in [the plan](GrevirRepositoryPlan.md).

| Repository | Mapped destination files | Shown |
| --- | ---: | ---: |
| grevir-arduino | 12 | 12 |
| grevir-arduino-avr | 43 | 30 |
| grevir-arduino-esp32 | 3 | 3 |
| grevir-avr | 627 | 30 |
| grevir-base | 29 | 29 |
| grevir-core | 12 | 12 |
| grevir-encoder | 2 | 2 |
| grevir-esp32 | 1 | 1 |
| grevir-fastled | 5 | 5 |
| grevir-packet | 5 | 5 |
| grevir-peripherals | 13 | 13 |
| grevir-platforms | 0 | 0 |
| grevir-pulse-codec | 5 | 5 |
| grevir-pulse-io | 3 | 3 |
| grevir-registers | 7 | 7 |
| grevir-stepper | 3 | 3 |
| grevir-test-support | 10 | 10 |
| grevir-time | 5 | 5 |
| grevir-tools | 0 | 0 |

## grevir-arduino

Showing 12 of 12 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `examples/Arrays/Arrays.ino` | `ardOinus/examples/Control/ArdOArrays/ArdOArrays.ino` |
| `examples/BareMinimum/BareMinimum.ino` | `ardOinus/examples/Basics/ArdoBareMinimum/ArdoBareMinimum.ino` |
| `examples/BetterBlink/BetterBlink.ino` | `ardOinus/examples/Basics/ArdoBetterBlink/ArdoBetterBlink.ino` |
| `examples/Blink/Blink.ino` | `ardOinus/examples/Basics/ArdoBlink/ArdoBlink.ino` |
| `examples/DependentModules/DependentModules.ino` | `ardOinus/examples/Basics/ArdoDependentModules/ArdoDependentModules.ino` |
| `examples/DigitalInputSerial/DigitalInputSerial.ino` | `ardOinus/examples/Basics/ArdoDigitalInputSerial/ArdoDigitalInputSerial.ino` |
| `examples/Serial/Serial.ino` | `ardOinus/examples/Basics/ArdoSerial/ArdoSerial.ino` |
| `src/grevir/arduino/core.hpp` | `ardOinus/src/ardo_sys_defs.h` |
| `src/grevir/arduino/eeprom.hpp` | `ardOinus/src/ardo_eeprom.h` |
| `src/grevir/arduino/pwm.hpp` | `ardOinus/src/ardo_sys_defs.h` |
| `src/grevir/arduino/serial.hpp` | `ardOinus/src/ardo_sys_defs.h` |
| `tests/blink_test.cpp` | `ardOinus/devel/code/blink_test.cxx` |

## grevir-arduino-avr

Showing 30 of 43 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `examples/FixedFrequencyCounter/FixedFrequencyCounter.ino` | `ardOinus/examples/RawAvr/FixedFreqencyCounter/FixedFrequencyCounter.ino` |
| `extras/legacy/ardOinus/src/sys/boards/arduino/arduino_boards.h` | `ardOinus/src/sys/boards/arduino/arduino_boards.h` |
| `extras/legacy/boards/ardo_board_LilyPadUSB_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_LilyPadUSB_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_atmegang_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_atmegang_atmega168.h` |
| `extras/legacy/boards/ardo_board_atmegang_atmega8.h` | `ardOinus/src/sys/boards/arduino/ardo_board_atmegang_atmega8.h` |
| `extras/legacy/boards/ardo_board_bt_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_bt_atmega168.h` |
| `extras/legacy/boards/ardo_board_bt_atmega328.h` | `ardOinus/src/sys/boards/arduino/ardo_board_bt_atmega328.h` |
| `extras/legacy/boards/ardo_board_chiwawa_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_chiwawa_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_circuitplay32u4cat_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_circuitplay32u4cat_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_diecimila_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_diecimila_atmega168.h` |
| `extras/legacy/boards/ardo_board_diecimila_atmega328.h` | `ardOinus/src/sys/boards/arduino/ardo_board_diecimila_atmega328.h` |
| `extras/legacy/boards/ardo_board_esplora_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_esplora_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_ethernet_atmega328p.h` | `ardOinus/src/sys/boards/arduino/ardo_board_ethernet_atmega328p.h` |
| `extras/legacy/boards/ardo_board_fio_atmega328p.h` | `ardOinus/src/sys/boards/arduino/ardo_board_fio_atmega328p.h` |
| `extras/legacy/boards/ardo_board_gemma_attiny85.h` | `ardOinus/src/sys/boards/arduino/ardo_board_gemma_attiny85.h` |
| `extras/legacy/boards/ardo_board_leonardo_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_leonardo_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_leonardoeth_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_leonardoeth_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_lilypad_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_lilypad_atmega168.h` |
| `extras/legacy/boards/ardo_board_lilypad_atmega328.h` | `ardOinus/src/sys/boards/arduino/ardo_board_lilypad_atmega328.h` |
| `extras/legacy/boards/ardo_board_megaADK_atmega2560.h` | `ardOinus/src/sys/boards/arduino/ardo_board_megaADK_atmega2560.h` |
| `extras/legacy/boards/ardo_board_mega_atmega1280.h` | `ardOinus/src/sys/boards/arduino/ardo_board_mega_atmega1280.h` |
| `extras/legacy/boards/ardo_board_mega_atmega2560.h` | `ardOinus/src/sys/boards/arduino/ardo_board_mega_atmega2560.h` |
| `extras/legacy/boards/ardo_board_micro_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_micro_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_mini_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_mini_atmega168.h` |
| `extras/legacy/boards/ardo_board_mini_atmega328.h` | `ardOinus/src/sys/boards/arduino/ardo_board_mini_atmega328.h` |
| `extras/legacy/boards/ardo_board_nano_atmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_nano_atmega168.h` |
| `extras/legacy/boards/ardo_board_one_atmega32u4.h` | `ardOinus/src/sys/boards/arduino/ardo_board_one_atmega32u4.h` |
| `extras/legacy/boards/ardo_board_pro_16MHzatmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_pro_16MHzatmega168.h` |
| `extras/legacy/boards/ardo_board_pro_16MHzatmega328.h` | `ardOinus/src/sys/boards/arduino/ardo_board_pro_16MHzatmega328.h` |
| `extras/legacy/boards/ardo_board_pro_8MHzatmega168.h` | `ardOinus/src/sys/boards/arduino/ardo_board_pro_8MHzatmega168.h` |

## grevir-arduino-esp32

Showing 3 of 3 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `extras/legacy/ardOinus/src/sys/ardo_sys_esp32.h` | `ardOinus/src/sys/ardo_sys_esp32.h` |
| `src/grevir/arduino_esp32/pin_bindings.hpp` | `ardOinus/src/sys/ardo_sys_esp32.h` |
| `src/grevir/arduino_esp32/selected_board.hpp` | `ardOinus/src/ardo_sys_defs.h` |

## grevir-avr

Showing 30 of 627 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `extras/legacy/ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_atmega328p_dev.h` |
| `extras/legacy/ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h` | `ardOinus/src/sys/mcu/avr/ardo_sys_avr_mcu_defs.h` |
| `extras/legacy/ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h` | `ardOinus/src/sys/mcu/avr/base/ardo_avr_base_timer.h` |
| `extras/legacy/mcu/ardo_supplemental_at43usb320_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at43usb320_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at43usb355_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at43usb355_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at76c711_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at76c711_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at86rf401_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at86rf401_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90c8534_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90c8534_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90can128_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90can128_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90can32_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90can32_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90can64_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90can64_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm161_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm161_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm1_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm1_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm216_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm216_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm2_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm2_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm2b_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm2b_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm316_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm316_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm3_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm3_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm3b_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm3b_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90pwm81_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90pwm81_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s1200_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s1200_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s2313_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s2313_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s2323_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s2323_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s2333_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s2333_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s2343_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s2343_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s4414_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s4414_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s4433_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s4433_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s4434_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s4434_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s8515_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s8515_dev.h` |
| `extras/legacy/mcu/ardo_supplemental_at90s8535_dev.h` | `ardOinus/src/sys/mcu/avr/ardo_supplemental_at90s8535_dev.h` |

## grevir-base

Showing 29 of 29 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/base/circular_buffer.hpp` | `ardOinus/src/circular_buffer.h` |
| `src/grevir/base/color.hpp` | `ardOinus/src/ardo_color_space.h` |
| `src/grevir/base/compat/array.hpp` | `ardOinus/src/setlx_array.h` |
| `src/grevir/base/compat/cassert.hpp` | `ardOinus/src/setlx_cassert.h` |
| `src/grevir/base/compat/config.hpp` | `ardOinus/src/setl_support.h` |
| `src/grevir/base/compat/cstddef.hpp` | `ardOinus/src/setlx_cstddef.h` |
| `src/grevir/base/compat/cstdint.hpp` | `ardOinus/src/setlx_cstdint.h` |
| `src/grevir/base/compat/cstdlib.hpp` | `ardOinus/src/setlx_cstdlib.h` |
| `src/grevir/base/compat/limits.hpp` | `ardOinus/src/setlx_limits.h` |
| `src/grevir/base/compat/tuple.hpp` | `ardOinus/src/setlx_tuple.h` |
| `src/grevir/base/compat/type_traits.hpp` | `ardOinus/src/setlx_type_traits.h` |
| `src/grevir/base/cyclic_int.hpp` | `ardOinus/src/setl_cyclic_int.h` |
| `src/grevir/base/diagnostics.cpp` | `ardOinus/src/setl_system.cpp` |
| `src/grevir/base/diagnostics.hpp` | `ardOinus/src/setl_system.h` |
| `src/grevir/base/int_scaler.hpp` | `ardOinus/src/setl_int_scaler.h` |
| `src/grevir/base/integers.hpp` | `ardOinus/src/setl_integers.h` |
| `src/grevir/base/memory_policy.hpp` | `ardOinus/src/setl_system.h` |
| `src/grevir/base/meta/tuple_algorithms.hpp` | `ardOinus/src/setl_tuple_helpers.h` |
| `src/grevir/base/meta/tuple_types.hpp` | `ardOinus/src/setl_tuple_helpers.h` |
| `src/grevir/base/meta/type_algorithms.hpp` | `ardOinus/src/setl_templ_utils.h` |
| `src/grevir/base/optional.hpp` | `ardOinus/src/setl_optional.h` |
| `src/grevir/base/type_for_size.hpp` | `ardOinus/src/type_for_size.h` |
| `src/grevir/base/utility.hpp` | `ardOinus/src/setl_utils.h` |
| `tests/circular_buffer_test.cpp` | `ardOinus/tests/circular_buffer_test.h` |
| `tests/color_test.cpp` | `ardOinus/devel/code/color_space_test.cxx` |
| `tests/cyclic_int_test.cpp` | `ardOinus/tests/setl_cyclic_int_test.h` |
| `tests/tuple_static_tests.cpp` | `ardOinus/src/setl_tuple_helpers.h` |
| `tests/type_algorithms_test.cpp` | `ardOinus/tests/setl_templ_utils_test.h` |
| `tests/type_for_size_test.cpp` | `ardOinus/tests/type_for_size_test.h` |

## grevir-core

Showing 12 of 12 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/core/allocation.hpp` | `ardOinus/src/ardo_timers.h` |
| `src/grevir/core/application.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/core/board.hpp` | `ardOinus/src/ardo_sys_defs.h` |
| `src/grevir/core/device_map.hpp` | `ardOinus/src/setl_device_map.h` |
| `src/grevir/core/module.hpp` | `ardOinus/src/ardo_params_modules.h` |
| `src/grevir/core/resource_checks.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/core/resource_claims.hpp` | `ardOinus/src/ardo_resources.h` |
| `src/grevir/core/resource_graph.hpp` | `ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h` |
| `src/grevir/core/singleton.hpp` | `ardOinus/src/ardo_singleton.h` |
| `tests/dependent_module_test.cpp` | `ardOinus/tests/dependent_module.cxx` |
| `tests/resource_claims_test.cpp` | `ardOinus/tests/ardOinus_test.h` |
| `tests/singleton_test.cpp` | `ardOinus/tests/ardo_singleton_test.h` |

## grevir-encoder

Showing 2 of 2 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/encoder/encoder.hpp` | `ardOQuadEncoder/src/ArdoQuadEncoder.h` |
| `tests/encoder_test.cpp` | `ardOQuadEncoder/devel/code/ardOQuadEncoderTest.cxx` |

## grevir-esp32

Showing 1 of 1 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/esp32/memory_policy.hpp` | `ardOinus/src/setl_system.h` |

## grevir-fastled

Showing 5 of 5 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `examples/FastLedQuadEncoder/FastLedQuadEncoder.ino` | `ardOFastLED/examples/FastLedQuadEncoder/FastLedQuadEncoder.ino` |
| `extras/legacy/examples/ParkLightsV2/ParkLightsV2.ino` | `ardOinus/examples/ParkLightsV2/ParkLightsV2.ino` |
| `extras/legacy/examples/ParkLightsV2/default_image.h` | `ardOinus/examples/ParkLightsV2/default_image.h` |
| `extras/legacy/examples/ParkLightsV2/parklights.h` | `ardOinus/examples/ParkLightsV2/parklights.h` |
| `src/grevir/fastled/strip.hpp` | `ardOFastLED/src/ArdoFastLED.h` |

## grevir-packet

Showing 5 of 5 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/packet/header.hpp` | `ardOnet/src/ardo_packet_reassembler.h` |
| `src/grevir/packet/manager.hpp` | `ardOnet/src/ardo_packet_reassembler.h` |
| `src/grevir/packet/reassembler.hpp` | `ardOnet/src/ardo_packet_reassembler.h` |
| `src/grevir/packet/sender.hpp` | `ardOnet/src/ardo_packet_reassembler.h` |
| `tests/packet_reassembler_test.cpp` | `ardOnet/tests/PaketReassemblerTest.cxx` |

## grevir-peripherals

Showing 13 of 13 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/peripherals/button_events.hpp` | `ardOinus/src/ardo_button_events.h` |
| `src/grevir/peripherals/gpio/debounce.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/peripherals/gpio/input.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/peripherals/gpio/interfaces.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/peripherals/gpio/output.hpp` | `ardOinus/src/ardOinus.h` |
| `src/grevir/peripherals/pwm_output.hpp` | `ardOinus/src/ardo_pwm_output.h` |
| `src/grevir/peripherals/sequencer.hpp` | `ardOinus/src/ardo_sequencer.h` |
| `src/grevir/peripherals/storage_region.hpp` | `ardOinus/src/ardo_eeprom.h` |
| `src/grevir/peripherals/time_poller.hpp` | `ardOinus/src/ardo_time_poller.h` |
| `src/grevir/peripherals/timer/requirements.hpp` | `ardOinus/src/ardo_timers.h` |
| `src/grevir/peripherals/timer/selection.hpp` | `ardOinus/src/ardo_timers.h` |
| `tests/gpio_application_test.cpp` | `ardOinus/tests/ardOinus_test.h` |
| `tests/timer_config_static_tests.cpp` | `ardOinus/src/ardo_timers.h` |

## grevir-platforms

No existing C++ files are assigned here. See the plan for the proposed
tooling/platform files.

## grevir-pulse-codec

Showing 5 of 5 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/pulse_codec/bits.hpp` | `ardOinus/src/pwe_serial.h` |
| `src/grevir/pulse_codec/decoder.hpp` | `ardOinus/src/pwe_serial.h` |
| `src/grevir/pulse_codec/encoder.hpp` | `ardOinus/src/pwe_serial.h` |
| `src/grevir/pulse_codec/waveform.hpp` | `ardOinus/src/pwe_serial.h` |
| `tests/codec_test.cpp` | `ardOinus/tests/pwe_serial_test.h` |

## grevir-pulse-io

Showing 3 of 3 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `extras/legacy/ardOinus/devel/code/gammil_repeater.cxx` | `ardOinus/devel/code/gammil_repeater.cxx` |
| `src/grevir/pulse_io/modules.hpp` | `ardOinus/src/pwm_serial_comms.h` |
| `tests/module_test.cpp` | `ardOinus/devel/code/pwe_test_module.cxx` |

## grevir-registers

Showing 7 of 7 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/registers/access.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `src/grevir/registers/apply.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `src/grevir/registers/bit_mapping.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `src/grevir/registers/bit_values.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `src/grevir/registers/fields.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `src/grevir/registers/selection.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `tests/bit_fields_test.cpp` | `ardOinus/tests/setl_bit_fields_test.cxx` |

## grevir-stepper

Showing 3 of 3 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `examples/StepperEncoder/StepperEncoder.ino` | `ardOStepper/examples/ardOStepperEncoder/ardOStepperEncoder.ino` |
| `src/grevir/stepper/stepper.hpp` | `ardOStepper/src/ardOStepper.h` |
| `tests/stepper_test.cpp` | `ardOStepper/src/devel/code/ardOStepperTest.cxx` |

## grevir-test-support

Showing 10 of 10 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `extras/legacy/ardOinus/devel/code/ardOinus_test.cxx` | `ardOinus/devel/code/ardOinus_test.cxx` |
| `include/grevir/test/arduino.hpp` | `ardOinus/src/mock_arduino.h` |
| `include/grevir/test/assert_that.hpp` | `ardOinus/src/assert_that.h` |
| `include/grevir/test/avr_registers.hpp` | `ardOinus/src/sys/mcu/avr/base/ardo_avr_base_register.h` |
| `include/grevir/test/framework.hpp` | `ardOinus/src/setl_test_framework.h` |
| `include/grevir/test/mock_core.hpp` | `ardOinus/src/sys/ardo_sys_mock.h` |
| `include/grevir/test/register_memory.hpp` | `ardOinus/src/setl_bit_fields.h` |
| `include/grevir/test/select_mock.hpp` | `ardOinus/src/ardo_sys_defs.h` |
| `src/arduino_globals.cpp` | `ardOinus/devel/code/emulate_arduino.cxx` |
| `src/avr_registers.cpp` | `ardOinus/tests/adro_avr_mock_registers.cxx` |

## grevir-time

Showing 5 of 5 mapped destination files.

| Planned file | Original source(s) |
| --- | --- |
| `src/grevir/time/interactive_scaling.hpp` | `ardOinus/src/setl_interactive_scaling.h` |
| `src/grevir/time/time.hpp` | `ardOinus/src/setl_time.h` |
| `src/grevir/time/units.hpp` | `ardOinus/src/setl_time_unit.h` |
| `tests/time_test.cpp` | `ardOinus/tests/setl_time_test.h` |
| `tests/units_test.cpp` | `ardOinus/tests/setl_time_unit_test.h` |

## grevir-tools

No existing C++ files are assigned here. See the plan for the proposed
tooling/platform files.

## Saved query

```sql
-- First 30 distinct planned destination files in each proposed repository.
-- Alphabetical by destination path; contributing source files stay together.
-- A repository with no mapped C++ destination appears with a NULL destination.
SELECT repository, total_destination_files, file_number, destination, sources
FROM first_30_files_per_repository
ORDER BY repository, file_number;
```

For full API notes for one repository:

```sql
SELECT destination, source, action, api_units, notes
FROM file_map
WHERE repository = 'grevir-core'
ORDER BY destination, source;
```

Other views: `repository_summary`, `repository_files`, and
`first_30_source_files_per_repository` (the corresponding source-file view).
