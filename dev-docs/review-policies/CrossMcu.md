# Grevir cross-MCU implementation and review policy

## Scope and policy selection

This is the shared policy in a **1+N model**: one cross-MCU policy, with one
additional policy per MCU architecture. Architecture supplements refine backend
implementation and target-specific evidence; they do not narrow the shared API's
contract for every other target.

| Code under review | Applicable policy |
| --- | --- |
| Generic Base, Time, Core, Peripherals and Registers APIs/implementations | This policy; consult relevant architecture supplements for target-dependent behavior |
| AVR backend (`grevir-avr/src`) | This policy plus [AVR](Avr.md) |
| Shared template instantiated for AVR | This policy for its public contract; AVR supplement for that instantiation's correctness and cost |
| Host tests, mocks and build tooling | This policy's evidence/reporting rules; do not impose firmware runtime budgets |
| Future MCU backend | This policy plus a dedicated architecture supplement when that backend is introduced |

AVR and 32-bit ESP32 are both intended consumers of the generic API. ESP32 is
not an excuse to ignore AVR portability, and AVR is not a reason to restrict all
generic operations to 8- or 16-bit arithmetic. The precise ESP32 architecture,
variant, ABI and floating-point capabilities must be identified when assessing
backend costs. Do not infer an FPU or its supported precision from “32-bit”.
No ESP32 backend policy is established by this document; add it when selecting
the backend. There are currently two policy documents: shared and AVR.

## Numeric correctness and representation

- Derive representation from the public value range, precision, units, overflow
  behavior and rounding contract. Preserve that contract across supported MCUs.
- `uint32_t`, `uint64_t` and floating-point types are not violations by themselves.
  Wider intermediates may be necessary for a correct result; smaller types may
  overflow or trigger different C++ promotions. Show why a cheaper representation
  preserves the required semantics before recommending it.
- Generic integer operations should not acquire incidental floating-point work.
  Distinguish that from an explicitly fractional algorithm, a caller-selected
  numeric type, or a documented floating default. Evaluate defaults as API choices
  against their intended use; do not declare a generic no-FPU ban.
- Do not replace fractional calculations with integer truncation, or shrink a
  literal's return type without checking its supported range and compatibility.
  A fixed-point alternative requires a scale, intermediate range and rounding
  definition. A fractional algorithm need not support arbitrary integer types
  unless its contract promises that support.
- Check expression types before assignment, including integer promotions,
  signedness, shifts and multiplication. A wide destination does not widen its
  operands retroactively. Shared code must be correct under relevant 16-bit and
  32-bit `int` models; host behavior alone cannot establish that.
- Keep target specializations, register widths and implementation policies behind
  the common capability API. Prefer explicit types, traits or policies to hidden
  target-dependent changes in public meaning.

## Runtime cost and evidence

Separate compile-time metadata, constant-evaluated calls, dynamic calls, retained
object storage and host-only code. `constexpr` permits runtime calls; it neither
proves elimination nor proves runtime overhead. Conversely, a wide template
parameter or type alias does not establish wide arithmetic in the firmware.

For a cost finding, identify a supported instantiation and operation where the
cost arises, the target to which it matters, and whether the operation is setup
work, a repeated update or an interrupt path. Check optimization where practical.
An isolated probe establishes behavior for that probe, not application-wide cost.
Do not invent cycle counts, flash/RAM savings, atomicity or helper-library calls
from host assembly/IR. Source reasoning may prove correctness defects without
target compilation; label the basis and limits of the evidence.

Do not flag allocations or virtual dispatch merely by keyword. Establish whether
they reach production code and are required, optional or accidentally introduced.
Review disabled compatibility/platform branches too, while stating which builds
select them. Honor existing validation holds; missing target measurements are an
evidence limitation, not permission to run deferred validation.

## Required reviewer instructions and finding format

Before reviewing, identify changed code, the shared/backend/host boundary,
applicable policies, intended targets and validation constraints. Any authorized
delegated reviewer must receive these details and the relevant policy documents.
This instruction does not itself authorize delegation.

Each reported finding must include:

1. File/line and concrete triggering input, configuration or instantiation.
2. Scope: shared contract, named target/backend, or host-only code.
3. Classification: correctness defect, demonstrated policy/cost violation,
   API/design tradeoff, or unverified concern.
4. Evidence and consequence, with severity based on impact and reachability.
5. Provenance: introduced by the change or inherited from Ardoinus/existing code.
6. A correction direction that preserves required range, precision and behavior,
   or the unresolved contract decision needed before selecting a correction.

Only actionable defects and demonstrated violations belong in the defect list.
Keep design tradeoffs and unverified concerns separate; do not attach defect
severity solely because a costly type appears. Record meaningful exclusions
briefly. Inherited defects remain actionable, but are not extraction regressions.

Examples: a generic 64-bit duration can be a legitimate range choice; an AVR
timer's integer path unconditionally converted to floating point is a backend
policy violation; a signed overflow is a correctness defect on each affected
configuration, regardless of whether that target executes the operation cheaply.
