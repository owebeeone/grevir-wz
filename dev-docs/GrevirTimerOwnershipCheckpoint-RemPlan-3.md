# Timer ownership checkpoint — focused closure of known roots

Status: **implemented; focused re-verdict pending**. Round 3 reviewed root
`542505d3bc03cc2af934a9fda65837daa92cb355`, Peripherals
`6553200b95f533ba77e5b123c9cc1f1896e1cc7f`, and AVR
`8a67186d5ad9cbcf59a2cb03836fa0349d7f103a`. Code and State both found
that the already-known complete-footprint invariant remained open, and State
also found a template-deduction route around the owner view. The State reviewer
subsequently corrected its classification: this is not a third new
architectural root cause; see the filed erratum. The P0 and NO-GO verdicts
remain unchanged until reviewer verification.

| Open finding | Correction | Closure evidence |
| --- | --- | --- |
| Code/State P0-1, role versus binding footprint collision | One `each_resource` traversal now supplies the full candidate footprint to both reservation and pairwise compatibility checks. | Constexpr role-versus-pin and role-versus-endpoint conflicts reject in both candidate orders; unrelated candidates select. Full host build and AVR/Arduino claim checks pass. |
| Code P1-1, public setup authority | Selected owner setup and all other register-writing setup entry points are private. Only the Core lifecycle parameter is a friend. | Positive dependency-ordered owner setup passes; negative probes reject direct foreign `setup_owner()` and `setup()` calls. |
| State P1-1, private view template rebinding | The module view is an opaque nested type returned from private `ViewStorage<Requests>`, so a foreign module cannot deduce a one-parameter view template from its `Plan` type. | A negative probe uses the reviewer's template-template partial-specialization pattern and fails; the positive owner binding compiles. |

This correction does not expand the supported use categories or claim silicon
evidence. A fresh focused review must verify the original counterexamples on
the settled tuple. If it finds a genuinely new architectural root cause, stop
this checkpoint lane under the review-loop cap.
