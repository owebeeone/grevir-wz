# Timer ownership checkpoint — closing exposed allocation routes

Status: **implemented; focused re-verdict pending**. The review of root
`66b751a771b186bbaf964ed42180eed8a729a9b0`, Peripherals
`0cf6a7d7d3358dcb3df9474c8e190d5217029e69`, and AVR
`9c1b570d43ed28e8d1d799786de8572b087d4cc5` closed the complete-footprint
P0 and the exact private-view rebinding trigger. Code and State found public
routes into the already-known writable-authority root, with no new
architectural root cause.

Core now keeps `Assemble::Requests`, `Claims`, `Allocation`, and `Modules`
private. The application exposes its read-only selected plan and claim types
for inspection. `RequestedModule::Bind::Impl` is private, and only
`RequestedModule` can call the selected timer parameter's private `runSetup()`;
the parameter retains a private AVR setup friendship for that lifecycle call.
Negative compiler probes attempt the reviewers' exact public Core wrapper,
bound implementation alias, and application allocation access; positive owner
setup, callback, and write probes remain. The host build and 187 CTests pass.

A fresh focused re-verdict must verify these counterexamples. This is bounded
closure of the previously recorded owner-authority root, not a new timer
feature or a claim of code-level security against deliberate reconstruction
of a backend allocation from public C++ templates.
