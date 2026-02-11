#!/usr/bin/env bash
# Profile encode_cli with Linux perf (if available).
set -e
BUILD_DIR="${BUILD_DIR:-build}"
if ! command -v perf &>/dev/null; then
  echo "perf not found; run on Linux with perf installed."
  exit 1
fi
echo "Profiling encode_cli (30 frames, 320x240)..."
perf record -g "$BUILD_DIR/encode_cli" -w 320 -h 240 -n 30 -o /tmp/profile_out.bin
perf report
