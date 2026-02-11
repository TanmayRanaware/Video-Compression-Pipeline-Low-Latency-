# Benchmarks

- **bench_motion_search**: Runs full-search motion estimation over a small frame (e.g. 320×240) for multiple iterations; reports MB/s.
- **bench_end_to_end**: Encodes N frames (synthetic source) and reports total time, effective fps, and kbps.

Run from `build/`:

```bash
./bench_motion_search
./bench_end_to_end
```

Use system profilers (e.g. `perf`, VTune) and compiler flags (`-O3`, `-march=native`) for optimization. SAD and residual loops are candidates for SIMD.
