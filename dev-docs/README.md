# Grevir development notes

## Selected project name — 18 September 2026

**Grevir is the selected name for the new declarative embedded C++ library
project evolving from Ardoinus.** Its GWZ workspace is `grevir-wz`, initialized
at `/Users/owebeeone/limbo/grevir-wz`. This supersedes the earlier proposed
workspace name `ardoinus-wz`.

The development notes and AVR constants generator were moved from
`ardoinus/dev-docs/` into this directory on 18 September 2026. Existing `Ardo…`
document filenames and historical references to Ardoinus are retained. Source
links point to the original Ardoinus checkout beside this workspace.

The project name is selected. `grevir-base` and `grevir-time` were extracted on
18 September; `grevir-core` followed on 20 September with native compilation and
expected-conflict checks. Core now also has ten passing behavioral mock tests,
which exposed and drove a fix for shared-dependency callback ordering. Broader
peripheral/MCU mock validation remains pending. Hardware validation is explicitly
on hold; the remaining `grevir-xxx` boundaries are described in the plan.

## Documents

- [Repository and C++ API extraction plan](GrevirRepositoryPlan.md)
- [Extraction progress and validation](GrevirExtractionProgress.md)
- [Embedded arithmetic review](GrevirEmbeddedArithmeticReview.md)
- [Cross-MCU implementation and review policy](review-policies/CrossMcu.md)
- [AVR implementation and review policy](review-policies/Avr.md)
- [Selected build and test framework and implementation plan](GrevirBuildAndTestPlan.md)
- [AVR compiler, simavr and Arduino validation plan (weftpi)](GrevirAvrValidationPlan.md)
- [First 30 files in each proposed repo — query results](GrevirFirst30Files.md)
- [Queryable file map (SQLite)](GrevirFileMap.sqlite)
- [Saved first-30-files query](GrevirFirst30Files.sql)
- [Original file-map import snapshot (CSV)](GrevirFileMap.csv)
- [Guiding principles](ArdoPrinciples.md)
- [Revamp goals and ideas](ArdoPlanIdeas.md)
- [Standalone packaging plan](ArdoPackagingPlan.md)
- [Original project survey and state of play](ArdoStateOfPlayAug26.md)
- [Embedded C++ ecosystem review](ArdoEcosystemReviewSep26.md)
- [Original AVR constants generator](avr_api_gen.py)
