#!/usr/bin/env python3
"""Run exactly one Arduino CLI compilation-database entry for the probe."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys


def main() -> int:
    database = Path(sys.argv[1])
    stem = sys.argv[2]
    matches = [entry for entry in json.loads(database.read_text())
               if Path(entry["file"]).name == stem]
    if len(matches) != 1:
        raise ValueError(f"expected one {stem} compile command, found {len(matches)}")
    entry = matches[0]
    return subprocess.run(entry["arguments"], cwd=entry["directory"],
                          check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
