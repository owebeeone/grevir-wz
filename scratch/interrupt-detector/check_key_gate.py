#!/usr/bin/env python3
"""Host syntax probe for stable-key interrupt binding gates."""

import os
from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parent
CXX = os.environ.get("CXX", "c++")


def main() -> None:
    for name, accepted in (("mapped", True), ("rogue", False), ("unbound", False)):
        source = ROOT / f"key_gate_{name}.cpp"
        result = subprocess.run(
            [CXX, "-std=c++23", "-fsyntax-only", str(source)],
            capture_output=True, text=True,
        )
        if (result.returncode == 0) != accepted:
            raise AssertionError(f"{name}: unexpected compiler result:\n{result.stderr}")
        print(f"PASS {name}: {'accepted' if accepted else 'rejected'}")


if __name__ == "__main__":
    main()
