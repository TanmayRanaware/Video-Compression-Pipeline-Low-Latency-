#include <util/CpuFeatures.h>

namespace telehealth {
namespace util {

bool has_sse4_1() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(__GNUC__) || defined(__clang__)
  __builtin_cpu_init();
  return __builtin_cpu_supports("sse4.1");
#else
  return false;
#endif
#else
  return false;
#endif
}

bool has_avx2() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(__GNUC__) || defined(__clang__)
  __builtin_cpu_init();
  return __builtin_cpu_supports("avx2");
#else
  return false;
#endif
#else
  return false;
#endif
}

}  // namespace util
}  // namespace telehealth
