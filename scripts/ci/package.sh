#!/usr/bin/env bash
# Packs the theme module as facet-theme-halloween-<version>-linux-<arch>.tar.gz
# whose top directory is the module directory (manifest.json, halloween.json),
# as expected by facet-core's `get.sh --plugin`. The module is only data, so
# the content is the same for every architecture; the archive is written once
# per name Facet looks for.
#   scripts/ci/package.sh <module-dir> <version> <out-dir> <arch>...
set -euo pipefail
dir="$1" version="$2" out="$3"
shift 3
root="$(cd "$(dirname "$0")/../.." && pwd)"
stage="$(mktemp -d)/theme-halloween"
mkdir -p "$stage" "$out"
install -m644 "$dir/manifest.json" "$dir/halloween.json" "$stage/"
for f in LICENSE README.md; do install -m644 "$root/$f" "$stage/$f"; done
for arch in "$@"; do
    name="facet-theme-halloween-$version-linux-$arch.tar.gz"
    tar -C "$(dirname "$stage")" -czf "$out/$name" theme-halloween
    echo "$out/$name"
done
