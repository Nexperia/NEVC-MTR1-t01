#!/usr/bin/env bash
# set_version.sh — Update the firmware version across all project files.
#
# Usage:
#   ./set_version.sh 1.2.0
#
# Files updated:
#   main/scpi_config.h   — FIRMWARE_VERSION (single C source of truth)
#   README.md            — Version badge
#   Doxyfile             — PROJECT_NUMBER
#   main/scpi.h          — Example response in documentation comment

set -e

VERSION="$1"

if [ -z "$VERSION" ]; then
    echo "Usage: $0 <version>  (e.g. $0 1.2.0)"
    exit 1
fi

if ! echo "$VERSION" | grep -qE '^\d+\.\d+\.\d+$'; then
    echo "Error: version must be in X.Y.Z format (e.g. 1.2.0)"
    exit 1
fi

ROOT="$(cd "$(dirname "$0")" && pwd)"

echo "Setting firmware version to $VERSION ..."

# 1. main/scpi_config.h — FIRMWARE_VERSION definition
sed -i -E "s|(#define FIRMWARE_VERSION\s+\")[^\"]+(\")|\1${VERSION}\2|" \
    "$ROOT/main/scpi_config.h"
echo "  Updated: main/scpi_config.h"

# 2. README.md — shields.io version badge
sed -i -E "s|(Version-)[0-9]+\.[0-9]+\.[0-9]+(-blue)|\1${VERSION}\2|" \
    "$ROOT/README.md"
echo "  Updated: README.md"

# 3. Doxyfile — PROJECT_NUMBER
sed -i -E "s|(PROJECT_NUMBER\s*=\s*\"NEVB-MTR1-t01-)[0-9.]+(\")|\1${VERSION}\2|" \
    "$ROOT/Doxyfile"
echo "  Updated: Doxyfile"

# 4. main/scpi.h — example response in Doxygen comment
sed -i -E "s|(NEVC-MTR1-t01-)[0-9]+\.[0-9]+\.[0-9]+|\1${VERSION}|g" \
    "$ROOT/main/scpi.h"
echo "  Updated: main/scpi.h"

echo "Done."
