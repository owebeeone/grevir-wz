# Interrupt ownership enforcement probe

Run from the workspace root:

```sh
python3 -B scratch/interrupt-ownership/check.py
```

This uses the host C++23 compiler and Grevir's real `AllocatedApplication`,
`RequestedModule`, `ResourceClaim` and conflict checker. It does not compile
for AVR or ESP32 and does not install a real interrupt.

| Probe | Expected observation |
| --- | --- |
| Two consumers depend on one UART source owner | Compiles and runs; the one mock backend setup executes once when application setup is called once. |
| Owners of distinct source IDs | Compiles and runs. |
| Two owners claiming source 0 in one application closure | Compilation fails with `Application has resource conflict`. |
| One allocation emits source 0 twice in its claims | Compilation fails with `Found resource conflict in same claim`. |
| Application setup called twice | Compiles and links; mock registration executes twice. The claim system cannot prove call count. |
| Two separate application closures in separate translation units each claim source 0 | Both compile and link; mock registration executes twice. Claims are checked per closure, not across the final program. |
| Two unguarded ESP-style calls with the same source argument | Links and calls the mock allocator twice. Linkers do not interpret function arguments. |
| The same two units emit a strong `grevir_irq_owner_uart0` symbol | Link fails on a duplicate symbol. This requires every Grevir owner to emit the guard. |
| One or two strong functions with the same AVR-like vector symbol | One links; two fail to link. These are host C-symbol analogues, not target ISR ABI tests. |

The result makes the enforcement boundary precise. This probe does not define
or discover an `on_interrupt` handler: its successful source claim therefore
also demonstrates that a claim alone does not imply any ISR demand or binding.
Compile-time claims work inside one complete application plan. A
source-specific strong symbol can catch
duplicate cooperating owners when both objects are linked. Neither mechanism
catches a foreign library that bypasses Grevir, two separate applications
without guards, or repeated setup calls. A production backend therefore needs
a canonical application setup path and a runtime registration check or
idempotence guard with a visible failure result. AVR vector and ESP32 runtime
behavior still require their own target validation.
