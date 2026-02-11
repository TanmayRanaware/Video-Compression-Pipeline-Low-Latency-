#pragma once

namespace telehealth {
namespace util {

/// Placeholder for CPU feature detection (SSE/AVX). Use for SIMD dispatch later.
bool has_sse4_1();
bool has_avx2();

}  // namespace util
}  // namespace telehealth
