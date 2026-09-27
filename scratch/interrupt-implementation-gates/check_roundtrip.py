#!/usr/bin/env python3
"""Exercise the target-compiled probe, JSON, mock emitter and final link."""

from __future__ import annotations

import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile


HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
TOOL = ROOT / "grevir-core/tools/grevir_irqgen/__main__.py"
VERIFY = ROOT / "grevir-core/tools/grevir_irqgen/verify.py"
CXX = os.environ.get("CXX", "clang++")
INCLUDES = ["-I.", "-Igrevir-base/src", "-Igrevir-core/src",
            "-Igrevir-peripherals/src", "-Igrevir-test-support/include"]


def run(*args: str, env: dict[str, str] | None = None,
        success: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(args, cwd=ROOT, env=env, capture_output=True, text=True)
    if (result.returncode == 0) != success:
        raise AssertionError(f"unexpected command result {result.returncode}: {args}\n"
                             f"{result.stdout}\n{result.stderr}")
    return result


def one_case(directory: Path, stem: str, header: str) -> None:
    output = directory / stem
    output.mkdir()
    object_path = output / "probe.o"
    run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_IRQ_PROBE=1", *INCLUDES,
        "-c", str(HERE / f"{stem}_record.cpp"), "-o", str(object_path))
    run(sys.executable, "-B", str(TOOL), "plan", "--object", str(object_path),
        "--out-dir", str(output), "--attempt", "first", "--backend", "mock",
        "--target", "mock_mcu", "--board", "mock_board",
        "--compiler", "scratch_compiler")
    plan_path = output / "grevir_generated_irq_plan_mock.json"
    first_plan = plan_path.read_bytes()
    plan = json.loads(first_plan)
    expected_demands = 0 if stem == "mock_zero" else (2 if stem == "mock_deferred_two" else 1)
    assert len(plan["demands"]) == expected_demands
    if stem == "mock_deferred_two":
        assert plan["deferred_context"] == {
            "capacity": 2, "policy": "host_mutex_v1"}
    run(sys.executable, "-B", str(TOOL), "emit", "--out-dir", str(output),
        "--attempt", "first", "--backend", "mock", "--compiler",
        "scratch_compiler", "--application-header", header)
    ready = output / "grevir_irq_ready.json"
    assert ready.exists()
    generated = output / "grevir_generated_irq_bindings_mock.cpp"
    verification = (sys.executable, "-B", str(VERIFY), "--out-dir", str(output),
                    "--backend", "mock", "--target", "mock_mcu",
                    "--board", "mock_board", "--compiler", "scratch_compiler")
    run(*verification, "--attempt", "first")
    if stem == "mock":
        shared = json.loads(first_plan)
        shared["bindings"][0]["shared_source"] = True
        unsigned = {key: value for key, value in shared.items()
                    if key != "fingerprint"}
        shared["fingerprint"] = hashlib.sha256(
            (json.dumps(unsigned, sort_keys=True, indent=2,
                        ensure_ascii=True) + "\n").encode()).hexdigest()
        plan_path.write_text(json.dumps(shared, sort_keys=True, indent=2) + "\n")
        run(*verification, success=False)
        plan_path.write_bytes(first_plan)
        for path in (plan_path, output / "grevir_generated_irq_bindings_mock.hpp",
                     generated, ready, output / ".grevir_irq_attempt"):
            original = path.read_bytes()
            path.write_bytes(original + b"x")
            run(*verification, success=False)
            path.write_bytes(original)
        marker = json.loads(ready.read_text())
        marker["emitter"] = "stale_emitter"
        ready.write_text(json.dumps(marker, sort_keys=True, indent=2) + "\n")
        run(*verification, success=False)
        marker["emitter"] = "grevir_irqgen_3"
        ready.write_text(json.dumps(marker, sort_keys=True, indent=2) + "\n")
        run(*verification)
    executable = output / "firmware"
    run(CXX, "-std=c++23", "-DHAS_STD_LIB=1",
        '-DGREVIR_GENERATED_IRQ_HEADER="grevir_generated_irq_bindings_mock.hpp"',
        *INCLUDES, f"-I{output}", str(HERE / f"{stem}_main.cpp"),
        str(generated), "-o", str(executable))
    run(str(executable))

    if stem == "mock":
        strict = [CXX, "-std=c++23", "-DHAS_STD_LIB=1",
                  '-DGREVIR_GENERATED_IRQ_HEADER="grevir_generated_irq_bindings_mock.hpp"',
                  *INCLUDES, f"-I{output}"]
        for negative in ("unbound_handler.cpp", "foreign_same_key.cpp"):
            run(*strict, "-fsyntax-only", str(HERE / negative), success=False)
        run(*strict, "-DGREVIR_TEST_DIFFERENT_PLAN=1", "-fsyntax-only",
            str(generated), success=False)
        run(*strict, str(HERE / "mock_main.cpp"), str(generated),
            str(generated), "-o", str(output / "duplicate"), success=False)
        run(sys.executable, "-B", str(TOOL), "emit", "--out-dir", str(output),
            "--attempt", "first", "--backend", "mock", "--compiler",
            "scratch_compiler", "--application-header",
            "scratch/interrupt-implementation-gates/mock_decl_only.hpp")
        run(*strict, str(HERE / "mock_decl_main.cpp"), str(generated),
            "-o", str(output / "undefined"), success=False)
    if stem == "mock_event":
        assert plan["demands"][0]["handler"] == "event"
        assert plan["demands"][0]["context"] == "isr"
        assert plan["demands"][0]["delivery"] == "direct"
        assert "dispatch_bound_interrupt" in generated.read_text()
        run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_IRQ_PROBE=1",
            *INCLUDES, "-fsyntax-only",
            str(HERE / "mock_event_dual_record.cpp"), success=False)
        run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_TEST_STALE_ROUTE=1",
            '-DGREVIR_GENERATED_IRQ_HEADER="grevir_generated_irq_bindings_mock.hpp"',
            *INCLUDES, f"-I{output}", "-fsyntax-only", str(generated),
            success=False)
    if stem == "mock_deferred_two":
        strict = [CXX, "-std=c++23", "-DHAS_STD_LIB=1",
                  '-DGREVIR_GENERATED_IRQ_HEADER="grevir_generated_irq_bindings_mock.hpp"',
                  *INCLUDES, f"-I{output}"]
        for define, diagnostic in (
                ("GREVIR_TEST_STALE_CAPACITY", "GREVIR_IRQ_STALE_EVENT_CONTEXT_CAPACITY"),
                ("GREVIR_TEST_STALE_POLICY", "GREVIR_IRQ_STALE_EVENT_CONTEXT_POLICY")):
            result = run(*strict, f"-D{define}=1", "-fsyntax-only",
                         str(generated), success=False)
            assert diagnostic in result.stderr, result.stderr
        changed = output / "changed"
        changed.mkdir()
        changed_object = changed / "probe.o"
        run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_IRQ_PROBE=1",
            "-DGREVIR_TEST_STALE_CAPACITY=1", *INCLUDES, "-c",
            str(HERE / "mock_deferred_two_record.cpp"), "-o", str(changed_object))
        run(sys.executable, "-B", str(TOOL), "plan", "--object", str(changed_object),
            "--out-dir", str(changed), "--attempt", "changed", "--backend", "mock",
            "--target", "mock_mcu", "--board", "mock_board",
            "--compiler", "scratch_compiler")
        changed_plan = json.loads((changed / plan_path.name).read_text())
        assert changed_plan["deferred_context"]["capacity"] == 1
        assert changed_plan["fingerprint"] != plan["fingerprint"]

    # The new attempt removes the old marker before doing any work. All four
    # failure points must leave the directory unready, even with old C++ files.
    for stage in ("before_json", "after_json", "after_header", "after_source",
                  "before_marker"):
        if stage in ("before_json", "after_json"):
            command = ("plan", "--object", str(object_path), "--out-dir",
                       str(output), "--attempt", stage, "--backend", "mock",
                       "--target", "mock_mcu", "--board", "mock_board",
                       "--compiler", "scratch_compiler")
        else:
            run(sys.executable, "-B", str(TOOL), "plan", "--object", str(object_path),
                "--out-dir", str(output), "--attempt", stage, "--backend", "mock",
                "--target", "mock_mcu", "--board", "mock_board",
                "--compiler", "scratch_compiler")
            command = ("emit", "--out-dir", str(output), "--attempt", stage,
                       "--backend", "mock", "--compiler", "scratch_compiler",
                       "--application-header", header)
        env = dict(os.environ, GREVIR_IRQGEN_FAIL_AT=stage)
        run(sys.executable, "-B", str(TOOL), *command, env=env, success=False)
        assert not ready.exists(), stage

    run(sys.executable, "-B", str(TOOL), "plan", "--object", str(object_path),
        "--out-dir", str(output), "--attempt", "final", "--backend", "mock",
        "--target", "mock_mcu", "--board", "mock_board",
        "--compiler", "scratch_compiler")
    assert plan_path.read_bytes() == first_plan
    tampered = json.loads(first_plan)
    tampered["board"] = "changed_board"
    plan_path.write_text(json.dumps(tampered, sort_keys=True, indent=2) + "\n")
    run(sys.executable, "-B", str(TOOL), "emit", "--out-dir", str(output),
        "--attempt", "final", "--backend", "mock", "--compiler",
        "scratch_compiler", "--application-header", header, success=False)
    assert not ready.exists()


def main() -> None:
    run(CXX, "-std=c++23", "-Igrevir-base/src", "-Igrevir-core/src",
        "-Igrevir-peripherals/src", "-fsyntax-only",
        str(HERE / "timer_allocator_probe.cpp"))
    run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_IRQ_PROBE=1",
        *INCLUDES, "-fsyntax-only", str(HERE / "inventory_probe.cpp"))
    run(CXX, "-std=c++23", "-Igrevir-base/src", "-Igrevir-core/src",
        "-Igrevir-peripherals/src", "-fsyntax-only",
        str(HERE / "undemanded_source_probe.cpp"))
    run(CXX, "-std=c++23", "-Igrevir-base/src", "-Igrevir-core/src",
        "-fsyntax-only", str(HERE / "duplicate_catalog.cpp"), success=False)
    with tempfile.TemporaryDirectory(prefix="grevir-irq-roundtrip-") as temp:
        directory = Path(temp)
        one_case(directory, "mock", "scratch/interrupt-implementation-gates/mock_app.hpp")
        print("PASS mock event probe, JSON, generated unit and final dispatch")
        one_case(directory, "mock_zero",
                 "scratch/interrupt-implementation-gates/mock_app_base.hpp")
        print("PASS zero-handler generated installer and transactional failures")
        one_case(directory, "mock_event",
                 "scratch/interrupt-implementation-gates/mock_event_app.hpp")
        print("PASS on_event activation, direct dispatch, dual rejection and stale route")
        one_case(directory, "mock_deferred_two",
                 "scratch/interrupt-implementation-gates/mock_deferred_two_app.hpp")
        print("PASS two-event deferred plan, capacity fingerprint and strict context gates")


if __name__ == "__main__":
    main()
