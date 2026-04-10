#!/usr/bin/env bash
# check_version.sh — Verify FIRMWARE_VERSION in scpi_config.h matches the expected version.
#
# Usage:
#   ./check_version.sh <expected-version>   (e.g. ./check_version.sh 1.2.0)
#
# Exits 0 on match, 1 on mismatch (suitable for CI gates).

set -e

EXPECTED="$1"

if [ -z "$EXPECTED" ]; then
    echo "Usage: $0 <expected-version>  (e.g. $0 1.2.0)"
    exit 1
fi

ROOT="$(cd "$(dirname "$0")" && pwd)"
CONFIG="$ROOT/main/scpi_config.h"

ACTUAL=$(grep -E '^#define FIRMWARE_VERSION' "$CONFIG" | sed -E 's/.*"([^"]+)".*/\1/')

if [ -z "$ACTUAL" ]; then
    echo "ERROR: Could not find FIRMWARE_VERSION in $CONFIG"
    exit 1
fi

if [ "$ACTUAL" != "$EXPECTED" ]; then
    echo "ERROR: Version mismatch!"
    echo "  Tag expects : $EXPECTED"
    echo "  Code defines: $ACTUAL"
    echo ""
    echo "Update FIRMWARE_VERSION in main/scpi_config.h (or run set_version.sh $EXPECTED) before tagging."
    exit 1
fi

echo "OK: FIRMWARE_VERSION matches tag ($ACTUAL)"
