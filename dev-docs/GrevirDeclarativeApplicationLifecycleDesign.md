# Declarative application lifecycle

Status: design proposal, 28 September 2026. This document defines the intended
contract; it does not claim that the current runners implement it. It applies
the [Declarative API Evaluator](../../bizscad/dev-docs/DeclarativeApiEvaluatorV2.md)
(internally v3) as a design instrument, alongside the Grevir
[dos](GrevirDeclarativeIntegrationDos.md) and
[don't-dos](GrevirDeclarativeIntegrationDontDos.md).

## Problem and design decision

`ApplicationSpec<Board, Descriptors...>` already derives a module closure for
interrupt discovery. `grevir::AllocatedApplication` separately binds modules to
an allocation and runs their setup and loop callbacks. The current
`grevir::interrupt::Application<Spec>` starts interrupt hardware through Board
hooks, but does not run those modules. The interrupt examples' `setup_modules()`
hooks are empty, and the interrupt runner has no `runLoop()`. The current
single `StartResult` and application-wide `event::stop<Spec>()` also cannot
describe or preserve partial success across independent owners.

The preferred design is one public `grevir::Application<Spec>` whose input is
the application specification. It derives one canonical integration plan from
the module closure, common and resident-target constraints, device facts, and
detected event handlers. The plan supplies module order, resource ownership,
complete peripheral configurations, and interrupt bindings. The application
runner orders the selected owners; each owner applies its own configuration.
The current allocated-module and interrupt runners become implementation
components of that one runner. A facade that merely calls both existing
runners would leave two selections and two lifecycle authorities, so it would
not meet this contract.

There is no backward-compatibility requirement for the existing portable
timer or interrupt runner APIs.

## Author surface and denotation

The author declares modules, their dependencies and requirements, any
constraints that distinguish this application, the board/target, and handlers
for chosen logical events. The author does not declare a second startup list,
timer-to-ISR table, or per-module call sequence. Conceptual spelling:

```cpp
using AppSpec = grevir::ApplicationSpec<Board, Ota, DriveTimer, Motor>;
using App = grevir::Application<AppSpec>;

void setup() {
  App::start();
}

void loop() {
  App::runLoop();
}
```

The two entry calls are integration with Arduino's required functions, not a
second description of the module graph. `runLoop()` must advance a pending
nonblocking start, expose the settled report, dispatch only active deferred
routes in the selected main-loop context, and run only active module loops.
It never treats `in_progress` as success. Applications can query a fixed-size,
stable-keyed startup report; they need not duplicate the runner to log or
react to failure. A target-specific thread context remains a declared policy,
not an implicit consequence of which task first calls a method.

For a valid description, the meaning is independent of the engine's incidental
iteration order:

1. The transitive dependency closure contains exactly the declared instances
   and their required dependencies. Each instance and owned resource has one
   stable identity. Reordering independent declarations does not change the
   selected plan or observable setup order.
2. Each selected physical peripheral is allocated to one module owner. That
   owner may expose its timer functions to dependent modules, but allocation
   does not silently share the timer between owners. The owner has one complete
   configuration whose offered interrupts and other functions are jointly
   legal.
3. A dependency completes setup before its dependent begins setup. For an
   interrupt-capable owner, its selected provider prepares and installs the
   binding while masked; the owner module's setup then runs. Routes are
   activated only after setup of all modules that can be reached by their
   handlers has completed. The conservative default is to finish the entire
   eligible module setup pass before activating any route. A module that needs
   an active interrupt during setup needs an explicit later readiness phase;
   it cannot silently rely on early activation. This order is derived from
   the dependency graph and owner plan, not from a Board hook or declaration
   order.
4. A selected owner's configuration or registration failure leaves its route
   disabled and its acquired resources cleaned up by that owner. The owner
   and transitive dependents are inactive. Independent modules still complete
   setup and can run their loops; an OTA module does not disappear because a motor timer
   failed. The report names the failed owner, phase, cause, cleanup outcome,
   and skipped dependents. An aggregate success means every required owner
   started; a partial result never masquerades as success.
5. Repeated startup is idempotent with respect to hardware effects. A call
   during an ESP32 startup transaction returns `in_progress` without waiting;
   after completion it replays the settled report. Loop service runs in its
   declared context. Deferred dispatch and module loop callbacks are bounded
   and ordered by an explicit policy, rather than by incidental container
   order.

This contract covers failures reported by Grevir-owned provider operations.
Current arbitrary `void runSetup()` module callbacks provide no failure
signal; this proposal does not pretend to infer one. A separate explicit
module-failure contract would be needed before such failures could govern
dependent activation. Likewise, an arbitrary ISR handler body can call code
outside its owner; C++ cannot infer those effects from the body. Cross-module
handler dependencies need an explicit, checked declaration or a type-level
access boundary before Grevir can claim that activation is safe earlier than
the conservative application-wide setup barrier.

## Static plan, runtime execution, and ownership

At constant evaluation, build the closure, collect requirements and event
demands, normalize identities, choose complete candidates, derive dependency
and owner order, and validate resource and binding conflicts. The public plan
must show each chosen owner and why it was selected. Missing capabilities,
cycles, ambiguous or duplicate identities, incompatible timer functions, and
unbound demanded events fail before effects. A generator may lower the checked
interrupt plan to AVR vector syntax or ESP32 registration code; it may not
choose a different owner or configuration.

At runtime, the runner coordinates phases, but provider methods own all
hardware effects. The minimum selected-owner protocol is conceptually
`prepare` (configure and install while masked), `activate` (enable after the
required setup barrier), and `cleanup` (undo only acquired effects). The exact C++
spelling remains open until the AVR timer, ESP32 timer and mock owner have each
demonstrated the same contract. Board supplies facts, reservations and context
policies. It does not interpret a selected timer name or directly program
that timer.

The event queue is prepared once before any selected interrupt can post. A
failure of one owner must not stop a queue serving a successful independent
owner. Route activity and cleanup therefore have owner scope even when queue
storage has application scope. A plan may select zero, one or many owners;
the runner must not index the first configuration as though it were the only
one. AVR, ESP32 and mock may implement distinct low-level effects while
preserving this lifecycle meaning.

## Evaluator application

This is a **design-stage** use of the evaluator. There is no frozen 30-intent
corpus, held-out split, independent oracle or executed mutant suite for this
new surface yet. Consequently this document assigns no numerical Δ, C,
regularizer or frontier score. In particular, a proposed static check is not
reported as a measured correctness result.

| Evaluator concern | Design requirement and evidence to obtain |
| --- | --- |
| Unit and gate | Score application composition (intent to canonical plan) separately from runtime lifecycle (plan to effects). Module callbacks are an imperative split-off. Declaration order is absent except explicit dependencies; the plan is inspectable data; the engine derives a nontrivial graph. |
| Δ.1 gap richness | Closure and allocation require global scope and search; lifecycle ordering is a feed-forward derivation. Record the actual passes and measure expansion ratio on an intent corpus, rather than asserting a number from one example. |
| Δ.2 generativity | The target is topology derivation: from module requirements and target capabilities, derive owners, order and bindings. A user-written `setup_modules()` or per-ISR registration table would lower this to manual wiring. |
| C correctness envelope | Write named gates for cycles, ownership conflicts, unsupported functions, stale generated bindings, inactive-target options and duplicate events. Distinguish these pre-effect errors from runtime registration failure and physical behavior. Exercise mutants before claiming a detection stage. |
| Seams | Evaluate closure-to-allocation, plan-to-generated-binding, plan-to-provider, and startup-to-loop separately. Especially test that a selected ISR cannot fire before modules it may reach are ready and that failure of one owner does not stop an independent route. |
| R1 semantics | Use the denotation above to state what a valid program means independently of the current runner's behavior. |
| R2 explanation | Expose stable plan identities, selected candidate reasons and a startup report keyed by owner, phase and blocked dependents. |
| R3 composition | Add a second timer owner, a shared dependent, independent OTA, and declaration permutations. Record every interaction rule; no hidden first-owner special case. |
| R4 escape containment | Provider-specific register and SDK operations stay in selected owners. A custom module callback cannot mutate allocation or binding authority through an invisible hook. |
| R5 independent oracle | Compare expected order and failure isolation against a separately written dependency-graph model and target capability facts. A test copied from runner implementation is not an oracle. |
| R6 coverage and R7 authoring cost | Freeze at least 30 exogenous application intents, with at least one-third held out, before numeric scoring. Include valid zero-, one- and many-owner cases, unused capabilities, failures and mixed MCU sections. Measure author declarations and edits against minimal intent. |

The live evaluator error classes include U4/U5 (references), U9/U18
(duplicates and identity collisions), U10 (ordering assumptions), U11/U12/U16
(seams and escapes), U17 (silently completed omissions), E-R1 (dependency
cycles), E-S1/E-S2/E-S3 (allocation), and E-L1/E-L2/E-L3 (faithful lowering,
dead declarations and target divergence). A Grevir lifecycle extension should
add a versioned class for activation before owner readiness and one for
incorrect failure propagation; those classes need concrete mutants and weights
before computing C. Runtime registration failure belongs in the report and in
target/mock validation, not in a claim that compilation made it impossible.

## Design gates before implementation is called complete

1. A mock program with two owners, a dependent and independent OTA proves the
   specified setup and loop behavior in success, one-owner failure and retry
   cases. Permuting independent declarations leaves its plan and execution
   order unchanged.
2. The same owner plan supplies configuration and interrupt bindings; removing
   a binding, changing the selected configuration after generation, or adding
   a conflicting event fails the enforced build gate before firmware effects.
3. The AVR owner uses extracted device facts and its typed timer facilities;
   the ESP32 owner uses its own backend. Both implement the same phases without
   Board register/SDK programming. Cross-target declarations remain parseable
   when a target-specific section is nonresident.
4. A failed owner leaves its ISR disabled and its dependents inactive while an
   independent module continues. Reentrant ESP32 startup remains nonblocking.
   Host behavior tests, target compilation and eventual hardware behavior are
   reported as distinct evidence; hardware validation remains on hold.

These gates justify a mock-first implementation, then target integration. The
first implementation should change the runner and selected-owner protocol,
not merely fill in `Board::setup_modules()`.
