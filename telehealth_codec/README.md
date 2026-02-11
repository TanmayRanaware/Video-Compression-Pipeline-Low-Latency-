# Telehealth Codec

A C++17 inter-frame video compression pipeline optimized for **low-latency, high-throughput telehealth streaming**. Educational implementation with a custom "H.264-ish" style codec (macroblocks, motion estimation, residuals, integer transform, entropy coding).

## Features

- **Input**: Raw RGB frames from synthetic generator or (with FFmpeg) from file/camera
- **Output**: Custom bitstream (`.bin`) and optional UDP streaming
- **Codec**: YUV420p, 16×16 macroblocks, P-frames with motion estimation (full/diamond search), motion compensation, 8×8 integer transform, quantization, zigzag + RLE + simple VLC entropy coding
- **Pipeline**: Bounded-queue stages (Capture → Convert → Encode → Packetize/Send) with drop-oldest backpressure
- **Streaming**: UDP packetization with reassembly and jitter buffer on receiver

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
# Optional: disable FFmpeg to use synthetic input only
# cmake .. -DTELECODEC_USE_FFMPEG=OFF
make -j4
```

### Options

- `TELECODEC_USE_FFMPEG` (ON): Use FFmpeg for file/camera input (requires libavcodec, libavformat, libavutil, libswscale)
- `TELECODEC_BUILD_TESTS` (ON): Build unit tests
- `TELECODEC_BUILD_BENCHMARKS` (ON): Build benchmarks
- `TELECODEC_BUILD_APPS` (ON): Build CLI apps

## Usage

### Encode to file (synthetic input)

```bash
./encode_cli -o output.bin -w 640 -h 480 -n 100
./encode_cli -i /path/to/video -o output.bin   # with FFmpeg
```

### Live stream sender / receiver

```bash
# Terminal 1: receiver
./live_stream_receiver -p 5000 -o received.bin

# Terminal 2: sender
./live_stream_sender -h 127.0.0.1 -p 5000 -n 300
```

### Decode (minimal; parses bitstream and dumps metadata)

```bash
./decode_cli -i output.bin -o decoded.yuv
```

## Tests and benchmarks

```bash
cd build
ctest --output-on-failure
./bench_motion_search
./bench_end_to_end
```

## Project layout

- `include/` — Public headers: `codec/`, `pipeline/`, `io/`, `util/`
- `src/` — Implementation
- `apps/` — `encode_cli`, `decode_cli`, `live_stream_sender`, `live_stream_receiver`
- `tests/` — Unit tests (YUV conversion, block iterator, motion search, bitstream roundtrip)
- `benchmarks/` — Motion search and end-to-end benchmarks
- `docs/` — Architecture and bitstream format

## License

Use as reference/education. Not a production codec.
