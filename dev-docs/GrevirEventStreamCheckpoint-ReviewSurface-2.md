# Grevir Event Stream checkpoint — Surface re-review

**Date:** 2026-09-28  
**Axis:** Surface, focused, peer-blind and read-only  
**Verdict:** **GO** — the original P2-1 contradiction is closed on this tuple.

| Repository | Reviewed commit |
| --- | --- |
| Root | `723df9ba7bd0452cd661c6c9e8b3bd6df2b8e1a0` |
| grevir-core | `6d09870d78b0628da4c6a0007c4a7d45e4ae5473` |
| grevir-test-support | `d8fdd2e86c07ff62d08e282fc7fdb9990cdf9fec` |
| grevir-peripherals | `4612cecdc99c22f326a0c1498846fea34c0c596c` |
| grevir-avr | `5f4feffa15110aac9bf27f0bc17c2ce668b003d6` |

All five commits matched at the start and end of review, and all five worktrees were clean. I inspected the remediation plan, the root docs diff from `2c2ea1e1b3326caf41f38acdb8f79c02b21b4895`, and the public interrupt guide. I read no implementation or peer report and ran no build or test.

## Prior-finding closure

| Finding | Status | Evidence |
| --- | --- | --- |
| P2-1 — shared queue text says a second firing coalesces even for Stream | **Closed** | `docs/guides/interrupts.md:83–86` now assigns coalescing to Elide and a separate record to Stream. A needed record is dropped with sticky overrun when the queue is full. This agrees with the Stream contract at lines 74–79. |
| P3-1 — linked runnable examples exercise Elide only | **Open, nonblocking** | The example section was unchanged. The remediation plan records this for a later example pass. |

## Changed-range analysis

The root docs diff changes only `docs/guides/interrupts.md:83–86`. For two firings before any dispatch, the revised guide yields an unambiguous trace:

| Route | Capacity | Publisher results | Callbacks after draining | Overrun |
| --- | ---: | --- | ---: | --- |
| Elide | 1 | `queued`, `coalesced` | 1 | Clear |
| Elide | 2 | `queued`, `coalesced` | 1 | Clear |
| Stream | 1 | `queued`, `full` | 1 | Set |
| Stream | 2 | `queued`, `queued` | 2 | Clear |

The result column applies to `post` or `post_from_isr`; the hardware entry uses the queue but has no caller to receive a result. The revised wording also preserves the guide’s distinction between accepted and dropped firings. I found no new finding in the changed range.

## Risks and next action

This is a documentation re-verdict on the original counterexample, not an implementation or target-runtime assessment. The P3 example gap remains for the later pass recorded in the remediation plan. The Surface P2 blocker needs no further correction on this tuple.
