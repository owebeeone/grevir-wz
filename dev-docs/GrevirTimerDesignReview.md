# Independent adversarial timer design review

Date: 21 September 2026. Scope: the uncommitted allocation design and worked API
examples, not a production implementation. The user explicitly requested an
independent adversarial review. A separate reviewer inspected the draft read-only;
the author assessed findings and revised the documentation.

Policies: [CrossMcu](review-policies/CrossMcu.md) for the shared contract and
[AVR](review-policies/Avr.md) for AVR integration boundaries. No AVR compiler,
hardware or device-specific performance validation was performed. No backward
compatibility with the unused portable API was required.

## Findings and disposition

### R1 — Resource compatibility was incomplete

Classification: shared-contract specification gap introduced by the new draft,
already acknowledged there; blocking before sharing integration, not a proven
implementation defect. Original locations: `GrevirTimerAllocationDesign.md`
lines 190 and 214–226 in the pre-review draft.

Counterexample: one group owns T0, and its endpoints use T0/A and T0/B. Treating
the identities as unrelated lets an external T0/A claim escape whole-timer
exclusion; treating every internal parent/child use as independent overlapping
ownership makes the valid group conflict with itself. Separate timers requiring
different settings of one shared divider also need a defined compatibility rule.

Revision: [resource compatibility rules](GrevirTimerAllocationDesign.md#proposed-resource-compatibility-rules)
now distinguish one owner with authorized internal endpoints from external
ancestor/descendant claims, preserve duplicate/range conflicts, require alias
normalization, and define agreement for shared configuration settings. Unknown
relationships cannot silently be assumed independent. The
[new compatibility examples](GrevirTimerApiExamples.md#8-ownership-and-simultaneous-duty-counterexamples)
exercise each distinction. These are proposed semantics; implementing the claim
adapter and obtaining user agreement remain separate work.

### R2 — Duplicate identities left diagnostic ordering ambiguous

Classification: P2 shared-contract omission introduced by the new draft.
Original locations: `GrevirTimerAllocationDesign.md` lines 232 and 278–283 in the
pre-review draft.

Counterexample: two `("left", "pwm")` declarations contain different invalid
numeric requirements. Sorting their errors only by identity can retain source
order and change the structured failure when the declarations are reversed.

Revision: validate active identity structure before request contents. Report one
duplicate-identity record per repeated identity with an occurrence count, ordered
by identity, and stop that phase. Later diagnostic ordering includes category and
normalized constraint/resource details. No declaration wins by position. The
[identity example](GrevirTimerApiExamples.md#3-reusable-modules-have-local-names-applications-name-instances)
now includes this failure case. C++ parsing/name errors and compiler trace layout
are outside the structured diagnostic guarantee.

### C1 — Make simultaneous duty independence explicit

Classification: clarification of the shared backend obligation, not a demonstrated
contradiction. Identified in the author's parallel walkthrough and assessed by the
independent reviewer. Original locations: design lines 206–208 and examples lines
206–209, 223 in the pre-review draft.

Two channel IDs can share an inseparable compare value. Each channel separately
may have fine duty granularity, yet 25% on A and 75% on B cannot coexist. The revised
contract requires the declared duty combinations to be attainable simultaneously
under one fixed candidate, with each endpoint update preserving the others'
selected steady duties. Unioning capabilities across configurations is prohibited.
Transition/rounding guarantees remain separate integration decisions.

### C2 — Qualify completeness versus work limits

Classification: nonblocking clarification of a stated prototype obligation.
The draft now distinguishes mathematical search completeness from operational
success under a work/compiler limit. Exhaustion is not unsatisfiability. A practical
supported problem envelope and budget remain to be established by a prototype.

## Evidence and limits

The reviewer found no counterexample to the successful-allocation ordering,
backtracking, inactive-target invariance or rational acceptance rules in the
examples. This is not proof of an implementation. Independently, the author checked
the written frequency/duty tables using exact rational arithmetic and enumerated
permutations of the two synthetic allocation examples; their stated canonical
assignments agree. No production tests were added or run for this document review.

The API spellings, explicit sharing policy, constraint intersection, frequency
tolerance and identity scheme remain recommendations. No review result promotes
them to user-approved decisions. Duty rounding, initial output state and setup
ordering remain integration gates; lazy target selection and search cost need
prototype evidence.

Focused independent recheck: completed. The reviewer confirmed that R1 and R2 are
addressed at design level, and that the simultaneous-duty and completeness
clarifications resolve their ambiguities. No new concrete contradiction was found
in the revised sections. This closes this bounded review loop; it does not certify
an implementation or settle the remaining user-facing design choices.
