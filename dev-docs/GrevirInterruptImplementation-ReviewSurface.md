# Interrupt binding implementation — SURFACE-AXIS REVIEW

## Evidence base
Read-only pre-commit audit of `docs/guides/interrupts.md`, both interrupt example directories, `docs/index.md`, `docs/supported.md`, compared with `docs/guides/pwm.md` and `docs/install.md`; read `AGENTS.md`, `AGENTS_GWZ.md`, and `dev-docs/review-policies/CrossMcu.md`. Ran top-level `python3 -B grevir-core/tools/grevir_irqgen/__main__.py --help` and its `arduino-build --help` (also `plan`/`emit` help). No builds, source reads, or edits. SHA-256 hashes of all 11 reviewed docs/example files matched at start and end. Review scope is public documentation and host tool help; MCU runtime and silicon behavior were not assessed.

## Findings

### P2 — Native mock path is not reproducible from public docs
- **Location/trigger:** `docs/guides/interrupts.md:100-106`; a first-day native user follows the advertised mock path. The only invocation is `grevir_add_interrupt_bindings(TARGET ... APPLICATION_HEADER ... BACKEND mock TARGET_ID ... BOARD ... COMPILER_ID ...)`; there is no complete mock application, board inventory, concrete CMake file, install sequence for its dependencies, or invocation to run it. `docs/index.md:17` links only the Uno and ESP32 examples.
- **Scope/classification/provenance:** Host-only documentation completeness defect introduced with the new interrupt guide; no AVR runtime cost claim.
- **Impact:** The claim that mock dispatch runs on macOS/Linux/Windows (`interrupts.md:117`) cannot be reproduced or adapted using the public interrupt docs without inventing the omitted contract values and application structure. This blocks the documented first-day mock workflow.
- **Remedy:** Add a complete public mock example under `/docs` with application header, Board constants/inventory, `CMakeLists.txt`, install/configure/build/run commands, and expected observable result; link it from the guide and index.
- **Closure test:** In a clean checkout, a reader can follow only the public docs to build and run the mock example on a named native platform, with no ellipses or guessed identifiers.

### P3 — The wrapper command is hidden from top-level help
- **Location/trigger:** `python3 -B grevir-core/tools/grevir_irqgen/__main__.py --help` reports `usage: grevir-irqgen [-h] {plan,emit} ...` and lists only `plan,emit`, while `docs/guides/interrupts.md:65,80` invokes `arduino-build`. `... arduino-build --help` does work and lists options.
- **Scope/classification/provenance:** Host-tool command-discoverability defect of the new wrapper.
- **Impact:** A user starting with conventional `--help` cannot discover the Arduino wrapper and may conclude the documented command is unsupported.
- **Remedy:** Expose `arduino-build` in top-level usage/help and add a short purpose and pointer to the guide; keep subcommand help descriptive enough to explain identity and directory options.
- **Closure test:** Top-level `--help` lists `arduino-build`; subcommand help explains what it builds and points to the guide.

### P3 — Both documented commands assume a private Arduino CLI location
- **Location/trigger:** `docs/guides/interrupts.md:66,81` sets `--arduino-cli "$HOME/.local/bin/arduino-cli"`; `docs/install.md:16-24` demonstrates a PATH-resolved `arduino-cli`. A reader with a normal PATH installation outside `~/.local/bin` copies either interrupt command.
- **Scope/classification/provenance:** Host-only copyability defect introduced with the new guide.
- **Impact:** Both otherwise complete wrapper invocations fail at CLI launch even when the documented installation flow works.
- **Remedy:** Resolve the executable from PATH, e.g. `--arduino-cli "$(command -v arduino-cli)"`, or explicitly state the private path is an environment-specific placeholder and give a portable command.
- **Closure test:** Copy the command with `arduino-cli` installed on PATH in another location; it reaches wrapper validation/probe without a path edit.

## Invariant analysis
The guide and examples consistently use stable event keys, named board/backend/target/compiler identities, and a handler visible in the application header. The documented scope is Uno Timer1 overflow, classic ESP32 TG0/T0, and mock; the support page preserves the simulation/compile-only/physical-hardware distinction. The guide correctly warns that bare Arduino CLI and IDE compilation skip generation and that output requires a ready marker. I found no Grevir release number in these docs. The C-style control-flow bodies in reviewed example files are braced. No source or disabled branches were inspected, so these are surface observations only.

## Risks / next action
The Uno wrapper command omits `compiler.cpp.extra_flags=-std=c++23` shown in `docs/install.md:21-22`; wrapper help does not state whether it adds that flag. I cannot classify this as a defect without inspecting implementation or running a build, both outside this audit. Have the implementation reviewer or a separate authorized wrapper check confirm the default and make the guide explicit. The ESP32 command’s compile status and silicon routing remain outside this review and the stated hardware hold.

## Verdict
**NO-GO for first-day public-doc usability** because the P2 mock workflow is non-reproducible from `/docs`. The P3 help and path issues should be closed in the same documentation/tool pass.
