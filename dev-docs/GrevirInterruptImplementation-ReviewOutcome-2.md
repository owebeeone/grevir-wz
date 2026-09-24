# Interrupt implementation pre-commit review outcome

Status of the second review's two structural P2 findings: **both closed in
the current uncommitted source**. At the reviewed snapshot, Board setup read
an interrupt binding even when no handler existed. It also allowed a timer
candidate to name an owner unrelated to the event's module instance. A
focused code recheck found both fixed. Boards now retain the solver's complete
selected configuration and enable an interrupt source only if a binding
exists. `validate_inventory()` checks the candidate's owner label against the
`instance` string in its event key. That event type is discovered through the
application module closure, but the string itself is declared by the API
user. The foreign-owner probe checks the label mismatch. This is an internal
metadata-consistency check, not a type-level proof of module ownership.

The state review also found an AVR startup flag clear and target sketches
that ignored startup failure. The AVR example now preserves the pending
overflow for delivery on enable. Both sketches enter an explicit failure path
if startup fails. A focused state recheck found both issues closed in source.
The surface review's CTest success-output wording is fixed. This is a
pre-commit record, not formal settled-tree acceptance.

Current validation: macOS Apple Clang and native Windows MSVC pass the mock
proofs or CMake smoke tests; Raspberry Pi GCC passes the mock round trip and
CMake smoke tests. The Pi also compiles and links the public Uno and classic
ESP32 examples, with and without handlers. Simavr enters the final Uno's
generated Timer1 overflow vector and the handler counter reaches one.
Physical AVR and ESP32 behavior is not claimed.
