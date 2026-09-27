# Grevir Timer Ownership Checkpoint — SURFACE-AXIS REVIEW

**Object:** Published timer ownership documentation and example  
**Baseline:** Root `09b954e2b9fe11f1df93ef278533e39afb81ca00`  
**Reviewed tuple:** Root `747ce149d027de8654ea8e8c908dd54e280897cf`; Core `c593678a36814f62655d1bbb976a94a9c42c97e3`; Peripherals `a89e6a8fa98c041f89a5d88eb72ff48a22574fa1`; AVR `a883814ace2203b08eb1186c18c137c8ef74eb8f`. These hashes matched at the start and end; the workspace was clean.  
**Date:** 2026-09-27  
**Axis:** SURFACE  
**Verdict:** **GO.** No P0–P2 surface finding. Two bounded P3 documentation findings remain.

## Prior-finding closure

| Prior finding | Disposition |
| --- | --- |
| P2: Two-use owner workflow absent | **Closed.** `docs/guides/pwm.md:9–69` declares `"left"` and `"right"` within one `timer::Own`, obtains both owner-local bindings, and writes both outputs. `docs/examples/pwm-host.cpp:38–72` checks shared TOP and both compare values. |
| P3: Evidence for this path unclear | **Closed.** `docs/supported.md:39–41` assigns the two-output example and dependency-order checks native host-mock evidence and explicitly excludes AVR recompilation, simavr and silicon evidence for this checkpoint. |

## Changed-range analysis

The changed guide and example now support a cold walkthrough of one owner with two synchronous Timer1 outputs. The guide distinguishes the owner identity `"motor"` from the internal output names `"left"` and `"right"` through the declaration and binding examples. It identifies PB1/PB2, ICR TOP, the 16-bit minimum, Timer1 pinning, Arduino Timer0 reservation, and the absence of an ESP32 PWM backend. The published host source checks TOP `15999` and compare values `3999` and `11999`.

**NEW ARCHITECTURAL root cause:** None found on the reviewed surface. The remaining findings concern reproduction instructions and evidence precision.

## 0 Evidence base

I read the specified root documentation, relevant API and installation pages, the prior surface review, the remediation plan, and the required cross-MCU and AVR review policies. I compared the public-page changes against the prior root. I did not read implementation or design code, build, run tests, or perform target validation. The stated 187-CTest result is lane-owner evidence, not independently verified in this review.

## 1 Findings

### P3 — The complete host example lacks a copyable build recipe

**Location:** `docs/examples/pwm-host.md:10–13`; related installation guidance at `docs/install.md:42–69`.

**Trigger and scope:** A first-day native user copies `docs/examples/pwm-host.cpp` and follows the instruction to build through an installed `grevir::avr` CMake target. This is a host example documentation issue.

**Classification and provenance:** Bounded reproducibility gap, inherited from the earlier host-example page.

**Evidence and impact:** The page names a target and “required Grevir package include paths” but supplies neither a consumer `CMakeLists.txt` nor commands that configure, build and run this particular source. The installation page’s concrete consumer recipe covers Base + Packet. The source itself is complete, but the documented steps do not let a new reader reproduce its result without discovering additional build details.

**Remedy and closure test:** Add a minimal consumer CMake recipe, including the required package discovery, target link and run command. Follow it from a clean installed prefix or documented workspace checkout and confirm that the example exits successfully.

### P3 — Native test counts have not been reconciled with checkpoint evidence

**Location:** `docs/supported.md:9–11`.

**Trigger and scope:** A reader uses the support table to cite the current native validation count.

**Classification and provenance:** Published evidence precision issue; the table’s 186-count text was unchanged in this remediation, while the lane owner reports 187 CTests for the current host build.

**Evidence and impact:** The table still says 186 cases for macOS, Raspberry Pi and Windows. This review cannot establish that 187 cases ran on every listed platform, so changing all three counts without platform records would also overstate evidence.

**Remedy and closure test:** Reconcile each row with that platform’s recorded CTest result. Update only verified counts, retaining 186 where that remains the latest platform evidence.

## 2 Invariant analysis

The public path presents whole-timer ownership with two internal PWM uses and owner-local bindings. It keeps AVR target constraints separate from inert ESP32 request options and does not claim an ESP32 PWM driver. The example’s memory-backed register policy and the support page consistently limit this checkpoint’s evidence to host behavior. The guide states that timer setup follows dependencies and precedes owner setup; this surface review does not independently verify the implementation.

## 3 Risks and next action

Publish a copyable host build recipe and reconcile platform-specific CTest counts. These are P3 documentation corrections and do not block the surface verdict. No silicon behavior, future event backend, or ESP32 timer backend is claimed by this review.
