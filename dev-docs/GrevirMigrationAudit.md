# Migration recordkeeping audit — 23 September 2026

This audit reconciles the extraction ledger with the current checkout and the
Arduino work retained on weftpi. It changes records and documentation, not runtime
implementations. At the start of this audit, Arduino, Arduino AVR and FastLED
were uncommitted locally; the subsequent GitHub synchronization includes them.
The later Pulse IO and full-build checkpoint is in
[GrevirExtractionProgress.md](GrevirExtractionProgress.md); figures below are this
audit's historical snapshot.

## What is recorded

| Package | Newly recorded assignments | Evidence | Remaining mapping scope |
| --- | ---: | --- | --- |
| grevir-arduino | 6: four native-checked headers, two target-compile records | Core/PWM/Serial/EEPROM native header and mock checks; retained BareMinimum/Blink target artifacts | Legacy blink harness and five other mapped examples remain planned |
| grevir-arduino-avr | 10 native-checked assignments | Uno/Nano/old-bootloader headers, pin mappings, selection and timer policy; native headers/claims and retained Uno/Nano sketch artifacts | Broader board archive and FixedFrequencyCounter example remain planned |
| grevir-fastled | 5: one native-checked header, one target-compile record, three archives | Strip mock/header/claim checks; retained FastLedQuadEncoder ELF; byte-identical ParkLightsV2 files | Archived ParkLightsV2 is not a supported, compiled example |

The new SerialHello, ReservedTimerFail and StripOn sketches and package-local
fixtures are new work, not automatic completion of differently scoped legacy
assignments. In particular, SerialHello does not port the timed Serial example.
The EEPROM assignment describes the Arduino byte backend; typed storage regions
are owned by Peripherals. The adapter uses EEPROM.write rather than update.
SerialIO currently supports port 0, not every historical serial operation.

The ledger has 802 assignments and 123 extraction/archive records: 117
`native_compile_checked`, three `target_compile_recorded`, three `retained_legacy`.
One additional assignment is explicitly superseded. Twenty-one missing records
were added and 18 stale destination hashes from the AVR compatibility changes
were refreshed. All 736 original source hashes are unchanged and all 123 recorded
destination hashes match the current files. SQLite integrity, foreign keys and
view row counts pass.

Schema version 2 adds two statuses:

- `target_compile_recorded`: target compilation has recorded evidence, whose host,
  date and audit limits appear in the notes. This is not native compilation or a
  claim that the compiler was rerun today.
- `retained_legacy`: an exact source archive, not compiled/supported runtime code.

`verified_on` is the last local audit date. The target execution date remains in
validation notes. Mapping revision 5 clarifies ownership/coverage; it does not
change destination paths. The CSV remains the original import snapshot.

```sql
SELECT repository, status, count(*) AS assignments
FROM migration_status
GROUP BY repository, status
ORDER BY repository, status;
```

## weftpi evidence

Host: `gianni@10.1.1.236`.
Workspace: `/home/gianni/git/grevir-wz`.

At audit, the remote root HEAD was `d78570a` and the local root HEAD was `26e9f4d`;
commit history alone therefore does not establish source equality. A read-only
comparison checked 178 files: Arduino/Arduino AVR/FastLED package files and the
available production source/include trees across members. All compared files
match locally. No remote-only files appeared in that compared set. This was not
an exhaustive comparison of build directories, Git metadata or all workspace docs.

The compiler reports `avr-g++ (GCC) 14.2.0`; installed FastLED reports 3.7.8.
Retained build options select `compiler.path=/usr/bin/`, C++23 and Arduino AVR
1.8.8. The following ELFs were inspected with avr-size:

| Retained sketch | Artifact under `avr-probe/arduino-cli/out/` | Flash bytes | RAM bytes |
| --- | --- | ---: | ---: |
| BareMinimum / Uno | `bareminimum/BareMinimum.ino.elf` | 442 | 9 |
| Blink / Uno | `blink-gcc14/Blink.ino.elf` | 1146 | 23 |
| Blink / Nano | `blink-nano/Blink.ino.elf` | 1146 | 23 |
| SerialHello / Uno | `serialhello/SerialHello.ino.elf` | 1458 | 194 |
| StripOn / Uno | `stripton/StripOn.ino.elf` | 3956 | 126 |
| FastLedQuadEncoder / Uno | `fastled-quad/FastLedQuadEncoder.ino.elf` | 5360 | 194 |

`reservedfail.log` contains the expected `Application has resource conflict.`
rejection. The retained remote host-test log contains 170 passes and zero failures;
its last run predates the additional 14 Arduino/FastLED cases. AVR firmware and
simavr host executables also exist in their documented build directories, but this
audit did not rerun the simulator or target compiler.

The [evidence manifest](GrevirWeftpiEvidence.json) records compared source hashes,
ELF hashes, size output, successful-build options and audit scope. Retained ELFs
and matching current source are evidence of the recorded work; this is not a fresh
reproducibility run. Silicon/electrical validation remains on hold.

## Local validation at this audit

All runtime targets rebuilt and all **184 CTest cases pass** on the Mac. The
Arduino, Arduino AVR and FastLED independent-header builds and claim probes pass.

At this audit, the default all-target build failed `grevir_timer_clock_checks`:
`check_timer_clock.cmake` passes only the AVR include directory, but the migrated
`clock.hpp` now includes `grevir/base/compat/cstdint.hpp`. The positive probe cannot
find that header. This is a compiler-probe dependency/configuration problem;
passing runtime tests do not make the all-target build green. No runtime source
was changed to conceal or bypass the failure.

The next planned work at this audit was to repair the standalone probe and
extract Pulse IO. The later checkpoint in GrevirExtractionProgress.md records
their completion.
