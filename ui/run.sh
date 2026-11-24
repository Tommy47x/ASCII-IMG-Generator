#!/usr/bin/env bash
# Build (if needed) and launch the ImGui desktop UI.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/ui/build"

if [[ ! -f "$BUILD/ascii_studio" ]]; then
    cmake -S "$ROOT/ui" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "$BUILD" -j"$(nproc 2>/dev/null || echo 2)"
exec "$BUILD/ascii_studio"
