#!/usr/bin/env python3
"""Check that selected bindings exactly match discovered interrupt handlers."""

import json
import os
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parent
GENERATOR = ROOT / "generate.py"
CATALOG = ROOT / "event_catalog.json"


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, check=True, capture_output=True, text=True)


def main() -> None:
    with TemporaryDirectory(prefix="grevir-irq-binding-") as dirname:
        output = Path(dirname)
        discovery = output / "discovery.cpp"
        object_file = output / "discovery.o"
        manifest = output / "manifest.json"
        run("python3", str(GENERATOR), "prepare", "--catalog", str(CATALOG),
            "--output", str(discovery))
        run(os.environ.get("CXX", "c++"), "-std=c++23", "-O2", "-I", str(ROOT),
            "-c", str(discovery), "-o", str(object_file))
        run("python3", str(GENERATOR), "inspect", "--catalog", str(CATALOG),
            "--object", str(object_file), "--output", str(manifest))

        base = json.loads((ROOT / "selected_plan.json").read_text())
        cases = (
            ("matched", base["bindings"], True),
            ("missing_handler_binding", [], False),
            ("unhandled_event_binding", [dict(base["bindings"][0],
                                               event_id=1002,
                                               cpp_type="MappedButNoHandler")], False),
        )
        for name, bindings, should_pass in cases:
            plan = output / f"{name}.json"
            plan.write_text(json.dumps(dict(base, bindings=bindings)))
            result = subprocess.run(
                ["python3", str(GENERATOR), "emit", "--catalog", str(CATALOG),
                 "--manifest", str(manifest), "--plan", str(plan),
                 "--out-dir", str(output / name)],
                capture_output=True, text=True,
            )
            if (result.returncode == 0) != should_pass:
                raise AssertionError(f"{name}: unexpected result: "
                                     f"{result.stdout} {result.stderr}")
            print(f"PASS {name}: {'accepted' if should_pass else 'rejected'}")


if __name__ == "__main__":
    main()
