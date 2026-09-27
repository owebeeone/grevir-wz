# Timer ownership checkpoint — remediation round 2

Status: **implemented; focused re-verdict pending**. The second peer-blind
review used root `747ce149d027de8654ea8e8c908dd54e280897cf`, Core
`c593678a36814f62655d1bbb976a94a9c42c97e3`, Peripherals
`a89e6a8fa98c041f89a5d88eb72ff48a22574fa1`, AVR
`a883814ace2203b08eb1186c18c137c8ef74eb8f`, and Arduino AVR
`0a5e262f77ce07f9e86cceb7d76184df24c008a5`.

Both Code P0-1 and State P0-1 identify one root cause: the common allocator
did not compare a reservation with the binding's endpoint and pin. The fix
checks the timer, every binding endpoint and nonzero pin, and extra exclusive
roles. The AVR translator no longer copies binding identities into extra
roles. Constexpr probes reserve the mock candidate's pin and endpoint, check
an unrelated reservation, and exercise reversed owner/candidate order. The
full host build and 187 CTests pass; AVR/Arduino claim probes still pass.

Code P1-1 is also accepted. The module-facing view is now a private nested
type of the allocation, available to `RequestedModule` for binding. The raw
AVR binding and its selected register writer are private; a module's public
view exposes only the owner-local use it declared. The positive motor binding
compiles, and negative probes reject both a foreign empty view and direct
attempts to name the raw binding or forge a new owner view.

Surface GO had two P3 findings. A consumer CMake project and configure/build/run
commands now accompany the two-output example; they were run against a fresh
local install prefix. The macOS row records this checkpoint's 187 passing
CTests; Raspberry Pi and Windows retain their prior verified 186 counts.

The common allocator and owner-view boundaries changed in this round, so a
fresh peer-blind re-review is required. Closure of blocking findings rests on
reviewers rechecking their original counterexamples, not this disposition.
