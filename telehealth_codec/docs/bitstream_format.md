# Bitstream format (MVP)

## File layout

1. **File header** (fixed size)
   - Magic: `0x54434F44` ("TCOD")
   - Version (uint16)
   - Width, height (uint16)
   - FPS (uint8)
   - Chroma format (0 = 4:2:0)
   - Reserved

2. **Per frame**
   - **Frame header**
     - Frame type (0 = I, 1 = P)
     - Frame ID, timestamp (us)
     - QP
     - MV payload size (bytes)
     - Coeff payload size (bytes)
   - **MV payload** (P-frames): Packed motion vectors (e.g. 2×16-bit per MB).
   - **Coeff payload**: Entropy-coded quantized coefficients (zigzag + RLE + VLC).

## Optional

- Checksum per frame for integrity.
- Decoder uses `BitstreamReader` to parse and reconstruct; roundtrip tests validate dimensions and basic consistency.
