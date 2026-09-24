#!/usr/bin/env python3
"""Run host-only positive and expected-failure interrupt ownership probes."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
CXX = shutil.which("c++")


def run(command: list[str], *, succeeds: bool, diagnostic: str = "") -> None:
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    output = result.stdout + result.stderr
    if succeeds and result.returncode != 0:
        raise RuntimeError(f"expected success: {' '.join(command)}\n{output}")
    if not succeeds and (result.returncode == 0 or diagnostic not in output):
        raise RuntimeError(
            f"expected failure containing {diagnostic!r}: "
            f"{' '.join(command)}\nexit={result.returncode}\n{output}"
        )


def main() -> int:
    if CXX is None:
        print("c++ compiler not found", file=sys.stderr)
        return 1
    core_flags = [
        CXX, "-std=c++23", "-DHAS_STD_LIB=1",
        f"-I{ROOT / 'grevir-core/src'}",
        f"-I{ROOT / 'grevir-base/src'}",
    ]
    with tempfile.TemporaryDirectory(prefix="grevir-irq-ownership-") as temp:
        out = Path(temp)
        for name in ("compile_ok", "compile_distinct_sources"):
            program = out / name
            run(core_flags + [str(HERE / f"{name}.cpp"), "-o", str(program)],
                succeeds=True)
            run([str(program)], succeeds=True)
            print(f"PASS {name}: application compiles and runs")

        run(core_flags + ["-fsyntax-only", str(HERE / "compile_duplicate_owners.cpp")],
            succeeds=False, diagnostic="Application has resource conflict")
        print("PASS duplicate owners: compile rejected")
        run(core_flags + ["-fsyntax-only", str(HERE / "compile_duplicate_selected.cpp")],
            succeeds=False, diagnostic="Found resource conflict in same claim")
        print("PASS duplicate selected claims: compile rejected")

        repeated = out / "repeated_setup"
        run(core_flags + [str(HERE / "repeated_setup.cpp"), "-o", str(repeated)],
            succeeds=True)
        run([str(repeated)], succeeds=True)
        print("PASS repeated setup: compile/link allow two registrations")
        separate = out / "separate_applications"
        run(core_flags + [str(HERE / "separate_app_a.cpp"),
                          str(HERE / "separate_app_b.cpp"),
                          str(HERE / "separate_app_main.cpp"),
                          "-o", str(separate)], succeeds=True)
        run([str(separate)], succeeds=True)
        print("PASS separate applications: cross-TU claims are not combined")

        registration_sources = [
            str(HERE / "registration_a.cpp"),
            str(HERE / "registration_b.cpp"),
            str(HERE / "registration_main.cpp"),
        ]
        unguarded = out / "unguarded"
        run([CXX, "-std=c++23", *registration_sources, "-o", str(unguarded)],
            succeeds=True)
        run([str(unguarded)], succeeds=True)
        print("PASS unguarded ESP-style calls: link succeeds and calls twice")
        run([CXX, "-std=c++23", "-DGREVIR_IRQ_STRONG_GUARD=1",
             *registration_sources, "-o", str(out / "guarded")],
            succeeds=False, diagnostic="grevir_irq_owner_uart0")
        print("PASS strong source guard: duplicate link rejected")

        vector_a = str(HERE / "vector_a.cpp")
        vector_b = str(HERE / "vector_b.cpp")
        vector_main = str(HERE / "vector_main.cpp")
        one_vector = out / "one_vector"
        run([CXX, "-std=c++23", vector_a, vector_main, "-o", str(one_vector)],
            succeeds=True)
        run([str(one_vector)], succeeds=True)
        print("PASS one strong vector: link succeeds")
        run([CXX, "-std=c++23", vector_a, vector_b, vector_main,
             "-o", str(out / "two_vectors")],
            succeeds=False, diagnostic="__vector_13")
        print("PASS two strong vectors: duplicate link rejected")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        raise SystemExit(1) from exc
