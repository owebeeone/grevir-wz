#!/usr/bin/env python3
"""Host proof of Grevir interrupt discovery and AVR ISR source generation.

The catalog and selected plan are fixtures for two separate Grevir compiler
products: known logical events, then allocation after handler discovery.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


CPP_TYPE = re.compile(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*")
CPP_IDENTIFIER = re.compile(r"[A-Za-z_]\w*")
# The prototype has only verified the spelling of this one device vector.
SUPPORTED_VECTORS = {"TIMER1_OVF_vect"}
PROBE = re.compile(
    r"(?:void\s+)?grevir_irq_probe<"
    r"(\d+)(?:u|ul|ull)?,\s*(true|false)>\(\)(?: noexcept)?"
)
CATALOG_MARKER = re.compile(r"_?grevir_irq_catalog_([0-9a-f]{64})")


class GenerationError(Exception):
    pass


def read_json(path: Path, label: str) -> dict:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise GenerationError(f"cannot read {label}: {exc}") from exc
    if not isinstance(data, dict) or data.get("schema") != 1:
        raise GenerationError(f"{label} must use schema 1")
    return data


def read_catalog(path: Path) -> list[dict]:
    catalog = read_json(path, "event catalog")
    if catalog.get("target") != "atmega328p":
        raise GenerationError("this probe only models ATmega328P events")
    events = catalog.get("events")
    if not isinstance(events, list):
        raise GenerationError("event catalog needs an events array")
    seen_ids = set()
    seen_types = set()
    for event in events:
        if not isinstance(event, dict) or set(event) != {"event_id", "cpp_type"}:
            raise GenerationError("catalog event needs event_id and cpp_type")
        event_id = event["event_id"]
        cpp_type = event["cpp_type"]
        if type(event_id) is not int or event_id <= 0:
            raise GenerationError("event_id must be a positive integer")
        if not isinstance(cpp_type, str) or not CPP_TYPE.fullmatch(cpp_type):
            raise GenerationError(f"unsupported C++ event type: {cpp_type!r}")
        if event_id in seen_ids or cpp_type in seen_types:
            raise GenerationError(f"duplicate catalog event: {event_id}, {cpp_type}")
        seen_ids.add(event_id)
        seen_types.add(cpp_type)
    return sorted(events, key=lambda event: event["event_id"])


def catalog_digest(events: list[dict]) -> str:
    canonical = json.dumps(
        {"schema": 1, "target": "atmega328p", "events": events},
        sort_keys=True, separators=(",", ":")
    )
    return hashlib.sha256(canonical.encode("utf-8")).hexdigest()


def read_selected_plan(path: Path, catalog: list[dict], present_ids: set[int]) -> list[dict]:
    plan = read_json(path, "selected plan")
    if plan.get("target") != "atmega328p":
        raise GenerationError("selected plan target mismatch")
    bindings = plan.get("bindings")
    if not isinstance(bindings, list):
        raise GenerationError("selected plan needs a bindings array")
    catalog_by_id = {event["event_id"]: event for event in catalog}
    seen_ids = set()
    seen_bindings = set()
    seen_sources = set()
    seen_vectors = set()
    for binding in bindings:
        if not isinstance(binding, dict) or set(binding) != {
            "event_id", "binding_id", "cpp_type", "source", "vector"
        }:
            raise GenerationError(
                "binding needs event_id, binding_id, cpp_type, source and vector"
            )
        event_id = binding["event_id"]
        binding_id = binding["binding_id"]
        cpp_type = binding["cpp_type"]
        source = binding["source"]
        vector = binding["vector"]
        if type(event_id) is not int or event_id not in catalog_by_id:
            raise GenerationError(f"unknown selected event ID: {event_id!r}")
        if type(binding_id) is not int or binding_id <= 0:
            raise GenerationError("binding_id must be a positive integer")
        if cpp_type != catalog_by_id[event_id]["cpp_type"]:
            raise GenerationError(f"event {event_id} C++ type mismatch")
        if not isinstance(source, str) or not CPP_IDENTIFIER.fullmatch(source):
            raise GenerationError(f"invalid source: {source!r}")
        if not isinstance(vector, str) or not CPP_IDENTIFIER.fullmatch(vector):
            raise GenerationError(f"invalid vector: {vector!r}")
        if vector not in SUPPORTED_VECTORS:
            raise GenerationError(f"vector is not in the prototype's device map: {vector}")
        if event_id in seen_ids:
            raise GenerationError(f"duplicate selected event ID {event_id}")
        if binding_id in seen_bindings:
            raise GenerationError(f"duplicate binding ID {binding_id}")
        if source in seen_sources:
            raise GenerationError(f"conflicting physical source owner: {source}")
        if vector in seen_vectors:
            raise GenerationError(f"conflicting vector owner: {vector}")
        seen_ids.add(event_id)
        seen_bindings.add(binding_id)
        seen_sources.add(source)
        seen_vectors.add(vector)
    if seen_ids != present_ids:
        raise GenerationError(
            f"allocation/handler mismatch: missing={sorted(present_ids - seen_ids)}, "
            f"unhandled={sorted(seen_ids - present_ids)}"
        )
    return sorted(bindings, key=lambda binding: binding["event_id"])


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(content)
        os.replace(temp_name, path)
    finally:
        if os.path.exists(temp_name):
            os.unlink(temp_name)


def discovery_source(events: list[dict]) -> str:
    digest = catalog_digest(events)
    lines = [
        "// Generated discovery probe; never link this object into firmware.",
        "#define GREVIR_IRQ_DISCOVERY 1",
        '#include "api.hpp"',
        '#include "handler.hpp"',
        "",
        "template <class Event>",
        "concept HasHandler = requires { on_interrupt<Event>(); };",
        "",
        "template <unsigned EventId, bool HandlerPresent>",
        "void grevir_irq_probe() noexcept;",
        f'extern "C" void grevir_irq_catalog_{digest}() noexcept;',
        "",
        "int main() {",
        f"  grevir_irq_catalog_{digest}();",
    ]
    for event in events:
        cpp_type = event["cpp_type"]
        lines.extend([
            f"  grevir_irq_probe<{cpp_type}::event_id,",
            f"                   HasHandler<{cpp_type}>>();",
        ])
    lines.extend(["  return 0;", "}", ""])
    return "\n".join(lines)


def inspect_object(object_path: Path, nm: str, cxxfilt: str, digest: str) -> dict[int, bool]:
    try:
        nm_result = subprocess.run(
            [nm, "-u", str(object_path)], check=True, capture_output=True, text=True
        )
        symbols = [line.split()[-1] for line in nm_result.stdout.splitlines() if line.split()]
        found_digests = [
            match.group(1) for symbol in symbols
            if (match := CATALOG_MARKER.fullmatch(symbol)) is not None
        ]
        if found_digests != [digest]:
            raise GenerationError(
                f"discovery object catalog fingerprint mismatch: "
                f"expected={digest}, found={found_digests}"
            )
        filt_result = subprocess.run(
            [cxxfilt], input="\n".join(symbols) + "\n", check=True,
            capture_output=True, text=True
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise GenerationError(f"symbol inspection failed: {exc}") from exc

    probes = {}
    for symbol in filt_result.stdout.splitlines():
        if "grevir_irq_probe" not in symbol:
            continue
        match = PROBE.fullmatch(symbol.strip())
        if match is None:
            raise GenerationError(f"unrecognized probe symbol: {symbol}")
        event_id = int(match.group(1))
        if event_id in probes:
            raise GenerationError(f"duplicate probe for event ID {event_id}")
        probes[event_id] = match.group(2) == "true"
    return probes


def manifest_from_probes(events: list[dict], probes: dict[int, bool]) -> dict:
    expected = {event["event_id"] for event in events}
    actual = set(probes)
    if expected != actual:
        raise GenerationError(
            f"probe set mismatch: missing={sorted(expected - actual)}, "
            f"unexpected={sorted(actual - expected)}"
        )
    return {
        "schema": 1,
        "target": "atmega328p",
        "catalog_sha256": catalog_digest(events),
        "handled_event_ids": sorted(event_id for event_id, present in probes.items() if present),
    }


def read_manifest(path: Path, catalog: list[dict]) -> set[int]:
    manifest = read_json(path, "handler manifest")
    if manifest.get("target") != "atmega328p":
        raise GenerationError("handler manifest target mismatch")
    if manifest.get("catalog_sha256") != catalog_digest(catalog):
        raise GenerationError("handler manifest/catalog fingerprint mismatch")
    ids = manifest.get("handled_event_ids")
    if not isinstance(ids, list) or any(type(event_id) is not int for event_id in ids):
        raise GenerationError("handler manifest needs integer handled_event_ids")
    known = {event["event_id"] for event in catalog}
    if len(ids) != len(set(ids)) or not set(ids) <= known:
        raise GenerationError("handler manifest has duplicate or unknown event IDs")
    return set(ids)


def detector_header(bindings: list[dict]) -> str:
    lines = [
        "// Generated from the current handler manifest and selected plan.",
        "#pragma once",
        '#include "api.hpp"',
        "",
    ]
    for binding in bindings:
        lines.extend([
            "template <>",
            f"struct DetectorImpl<{binding['cpp_type']}> {{",
            f"  static constexpr unsigned binding_id = {binding['binding_id']}u;",
            "};",
            "",
        ])
    return "\n".join(lines)


def isr_source(bindings: list[dict]) -> str:
    lines = [
        "// Generated AVR entries; host mocks require an explicit opt-in.",
        '#include "generated_detector.hpp"',
        '#include "handler.hpp"',
        "",
        "#if defined(__AVR__)",
        "#include <avr/interrupt.h>",
    ]
    for binding in bindings:
        lines.extend([
            f"ISR({binding['vector']}) {{",
            f"  on_interrupt<{binding['cpp_type']}>();",
            "}",
            "",
        ])
    lines.append("#elif defined(GREVIR_IRQ_HOST_MOCK)")
    for binding in bindings:
        lines.extend([
            f"extern \"C\" void grevir_mock_{binding['vector']}() noexcept {{",
            f"  on_interrupt<{binding['cpp_type']}>();",
            "}",
            "",
        ])
    lines.extend([
        "#else",
        '#error "generated ISR source needs AVR or GREVIR_IRQ_HOST_MOCK"',
        "#endif",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    prepare = subparsers.add_parser("prepare")
    prepare.add_argument("--catalog", type=Path, required=True)
    prepare.add_argument("--output", type=Path, required=True)
    inspect = subparsers.add_parser("inspect")
    inspect.add_argument("--catalog", type=Path, required=True)
    inspect.add_argument("--object", type=Path, required=True)
    inspect.add_argument("--output", type=Path, required=True)
    inspect.add_argument("--nm", default="nm")
    inspect.add_argument("--cxxfilt", default="c++filt")
    emit = subparsers.add_parser("emit")
    emit.add_argument("--catalog", type=Path, required=True)
    emit.add_argument("--manifest", type=Path, required=True)
    emit.add_argument("--plan", type=Path, required=True)
    emit.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()

    try:
        events = read_catalog(args.catalog)
        if args.command == "prepare":
            atomic_write(args.output, discovery_source(events))
        elif args.command == "inspect":
            probes = inspect_object(
                args.object, args.nm, args.cxxfilt, catalog_digest(events)
            )
            manifest = manifest_from_probes(events, probes)
            atomic_write(args.output, json.dumps(manifest, indent=2) + "\n")
            print(f"discovered {len(manifest['handled_event_ids'])} handler(s)")
        else:
            present_ids = read_manifest(args.manifest, events)
            bindings = read_selected_plan(args.plan, events, present_ids)
            atomic_write(args.out_dir / "generated_detector.hpp", detector_header(bindings))
            atomic_write(args.out_dir / "generated_isrs.cpp", isr_source(bindings))
            print(f"generated {len(bindings)} ISR binding(s)")
    except GenerationError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
