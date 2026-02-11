# Architecture

## Two layers

1. **Pipeline / orchestration** — Real-time, low-latency: bounded queues, stages, backpressure (drop oldest / adaptive QP).
2. **Codec core** — Pure encoding logic: testable, optimizable (SIMD-ready loops).

## Components

### IO

- **VideoSource**: Provides RGB frames (file/camera or synthetic). API: `read(FrameRGB&, FrameMeta&)`.
- **FileBitstreamSink**: Writes file header + frame payloads to `.bin`.
- **UdpPacketSink / UdpReceiver**: MTU-sized packets with stream_id, frame_id, packet_id, timestamp. Receiver reassembles; optional jitter buffer.

### Preprocess

- **YuvConverter**: RGB → YUV420p planar (aligned for SIMD).

### Partitioning

- **MacroblockPartitioner / BlockView**: 16×16 luma macroblocks; `for_each_macroblock` with block views (no copy). Boundary handling for right/bottom edges.

### Inter-frame core

- **MotionEstimation**: Full search or diamond search, SAD, configurable range. Returns `MotionVector` + cost.
- **MotionCompensation**: Integer-pel prediction from reference; boundary clamp.
- **Residual**: `current - predicted` (int16).
- **Transform**: 8×8 integer DCT-like forward/inverse.
- **Quantizer**: QP-based scale; quantize/dequantize 8×8.
- **EntropyCoder**: Zigzag, RLE of zeros, simple VLC; MV and coeff encoding.

### Bitstream

- **BitstreamWriter / BitstreamReader**: Bit-packed write/read; byte-align flush.
- **Container**: File header (magic, version, width, height, fps, chroma), per-frame header (type, frame_id, timestamp, QP, payload sizes), then MV and coeff payloads.

### Rate control and encoder

- **RateControl**: Choose QP from target bitrate; I-frame every GOP or on scene change.
- **Encoder**: Owns reference frame and all codec components; for each frame: I or P path, per-MB ME → MC → residual → transform → quant → entropy; outputs `EncodedFrame`.

### Pipeline

- **BoundedQueue**: Fixed capacity; push drops oldest when full.
- **Stage**: Thread running a process loop (e.g. pop from input queue, convert, push to output).
- **Pipeline**: Capture → Convert → Encode → (optional) Packetize/Send; configurable queue sizes and dimensions.

## Latency and throughput

- Frame budget (e.g. &lt; 33 ms per frame) is enforced by queue bounds and drop policy.
- Encoder can be parallelized by rows of macroblocks or slices in a later phase.
- SAD and residual loops are structured for SIMD (SSE/AVX) in future work.
