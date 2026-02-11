#!/usr/bin/env bash
# Encode a short synthetic sequence and optionally run decode.
set -e
BUILD_DIR="${BUILD_DIR:-build}"
OUT="${1:-/tmp/telehealth_sample.bin}"
mkdir -p "$(dirname "$OUT")"
echo "Encoding to $OUT ..."
"$BUILD_DIR/encode_cli" -w 320 -h 240 -n 30 -o "$OUT"
echo "Done. Output: $OUT"
if [[ -x "$BUILD_DIR/decode_cli" ]]; then
  "$BUILD_DIR/decode_cli" -i "$OUT" -o "${OUT%.bin}.yuv"
  echo "Decode (metadata) written to ${OUT%.bin}.yuv"
fi
