Read and follow `AGENTS_GWZ.md` before doing any work in this workspace.

For implementation and code review, read and follow
`dev-docs/review-policies/CrossMcu.md`. For AVR-specific code, also read and
follow `dev-docs/review-policies/Avr.md`. Shared code remains subject to the
cross-MCU API contract even when reviewing its AVR instantiations.

Use the 1+N policy model: one shared policy plus one supplement per supported
MCU architecture. Do not apply AVR cost assumptions to all generic Grevir APIs.
When assigning an authorized review, include the applicable policy paths,
code/target scope, and validation constraints in the reviewer's instructions.
