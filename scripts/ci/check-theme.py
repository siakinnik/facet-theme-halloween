#!/usr/bin/env python3
"""Checks the generated theme file against the limits of the core's reader
(Facet 0.8): size, the number of drawing operations, and that every colour
and operation is well formed.

    scripts/ci/check-theme.py halloween.json
"""
import json
import re
import sys

MAX_BYTES = 256 * 1024
MAX_OPS_PER_PLACE = 600
MAX_OPS_TOTAL = 6000
OPS = {"rect", "rrect", "circle", "ellipse", "ring", "line", "arc", "poly", "polyline", "text", "icon",
       "grad", "glow", "rglow"}
HEX = re.compile(r"^#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$")


def fail(msg):
    print(f"check-theme: {msg}", file=sys.stderr)
    sys.exit(1)


def main(path):
    raw = open(path, "rb").read()
    if len(raw) > MAX_BYTES:
        fail(f"{len(raw)} bytes, the limit is {MAX_BYTES}")
    theme = json.loads(raw)
    if theme.get("format") != 1:
        fail("format must be 1")
    for name, value in theme.get("palette", {}).items():
        if not HEX.match(value):
            fail(f"palette colour {name}: {value!r} is not #RRGGBB[AA]")
    total = 0
    places = {}
    for screen, hook in theme.get("screens", {}).items():
        for layer in ("under", "over"):
            places[f"screens/{screen}/{layer}"] = hook.get(layer, [])
    for part in ("tile", "card"):
        for layer in ("under", "over"):
            places[f"{part}/{layer}"] = theme.get(part, {}).get(layer, [])
    for where, ops in places.items():
        if len(ops) > MAX_OPS_PER_PLACE:
            fail(f"{where}: {len(ops)} operations, the limit is {MAX_OPS_PER_PLACE}")
        total += len(ops)
        for op in ops:
            if op.get("op") not in OPS:
                fail(f"{where}: unknown operation {op.get('op')!r}")
            for key in ("color", "from", "to"):
                v = op.get(key)
                if isinstance(v, str) and v.startswith("#") and not HEX.match(v):
                    fail(f"{where}: bad colour {v!r}")
    if total > MAX_OPS_TOTAL:
        fail(f"{total} operations in all, the limit is {MAX_OPS_TOTAL}")
    used = len([p for p in places.values() if p])
    print(f"ok: {path}: {len(raw)} bytes, {total} operations in {used} places")


if __name__ == "__main__":
    main(sys.argv[1])
