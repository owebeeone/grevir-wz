#!/usr/bin/env python3
"""Compile the two first implementation gates on an Apple Clang host."""

from __future__ import annotations

import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


HERE = Path(__file__).resolve().parent
WORKSPACE = HERE.parent.parent
sys.path.insert(0, str(WORKSPACE / "grevir-core/tools/grevir_irqgen"))
from object_section import ObjectSectionError, extract_interrupt_plan  # noqa: E402


EXPECTED = b"GIRQ\x01\x00\x18\x00mock\x00\x00\x00\x00motor.1\x00"
CXX = os.environ.get("CXX", "clang++")
INCLUDES = [
    "grevir-base/src", "grevir-registers/src", "grevir-core/src",
    "grevir-peripherals/src", "grevir-avr/src", "grevir-test-support/include",
    "scratch/interrupt-implementation-gates",
]


def run(*args: str) -> None:
    subprocess.run(args, cwd=WORKSPACE, check=True)


def rejected(data: bytes, reason: str) -> None:
    try:
        extract_interrupt_plan(data)
    except ObjectSectionError as error:
        if reason not in str(error):
            raise AssertionError(f"expected {reason!r}, received {error!r}") from error
    else:
        raise AssertionError(f"accepted object that should fail: {reason}")


def duplicate_coff(data: bytes) -> bytes:
    changed = bytearray(data)
    section_count = struct.unpack_from("<H", data, 2)[0]
    optional_size = struct.unpack_from("<H", data, 16)[0]
    symbol_offset, symbol_count = struct.unpack_from("<II", data, 8)
    strings_offset = symbol_offset + symbol_count * 18
    strings_size = struct.unpack_from("<I", data, strings_offset)[0]
    strings = data[strings_offset : strings_offset + strings_size]
    sections_offset = 20 + optional_size
    target_raw = None
    other_offset = None
    for index in range(section_count):
        offset = sections_offset + index * 40
        raw = data[offset : offset + 8]
        if raw.startswith(b"/"):
            name_offset = int(raw.split(b"\0", 1)[0][1:])
            name = strings[name_offset:].split(b"\0", 1)[0]
        else:
            name = raw.split(b"\0", 1)[0]
        if name == b".grevir_irq_plan":
            target_raw = raw
        elif name == b".data":
            other_offset = offset
    if target_raw is None or other_offset is None:
        raise AssertionError("COFF probe lacks expected sections")
    changed[other_offset : other_offset + 8] = target_raw
    return bytes(changed)


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="grevir-irq-proof-") as directory:
        output = Path(directory)
        for mode, definition in (("probe", ["-DGREVIR_IRQ_PROBE=1"]),
                                 ("strict", [])):
            exe = output / f"handler-allocation-{mode}"
            run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", *definition,
                *(f"-I{path}" for path in INCLUDES),
                str(HERE / "handler_allocation.cpp"), "-o", str(exe))
            run(str(exe))
        print("PASS probe and strict handler builds call selected PWM binding")
        run(CXX, "-std=c++23", "-DHAS_STD_LIB=1",
            "-Igrevir-base/src", "-Igrevir-core/src", "-fsyntax-only",
            str(HERE / "catalog_probe.cpp"))
        print("PASS event catalog is derived from module closure and order independent")
        run(CXX, "-std=c++23", "-DHAS_STD_LIB=1", "-DGREVIR_IRQ_PROBE=1",
            "-Igrevir-base/src", "-Igrevir-core/src", "-fsyntax-only",
            str(HERE / "demand_probe.cpp"))
        print("PASS visible handler adds one demand independent of module order")

        objects: dict[str, bytes] = {}
        for name, target in (("macho", None),
                             ("elf", "x86_64-unknown-linux-gnu"),
                             ("coff", "x86_64-pc-windows-msvc")):
            options = [] if target is None else ["-target", target]
            for stem in ("section_probe", "relocated_section"):
                object_path = output / f"{stem}-{name}.o"
                run(CXX, "-std=c++23", *options, "-c", str(HERE / f"{stem}.cpp"),
                    "-o", str(object_path))
                data = object_path.read_bytes()
                if stem == "section_probe":
                    if extract_interrupt_plan(data) != EXPECTED:
                        raise AssertionError(f"wrong {name} plan bytes")
                    objects[name] = data
                else:
                    rejected(data, "relocations")
            print(f"PASS {name} section bytes and relocation rejection")

        changed = objects["macho"].replace(b"__grevir_irq", b"__grevir_noo", 1)
        rejected(changed, "missing")
        rejected(duplicate_coff(objects["coff"]), "duplicated")
        for name, data in objects.items():
            rejected(data[:10], "")
        print("PASS missing, duplicate and truncated object rejection")


if __name__ == "__main__":
    main()
