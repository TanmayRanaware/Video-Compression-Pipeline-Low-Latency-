#include <util/AlignedAlloc.h>
#include <cstdlib>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

namespace telehealth {
namespace util {

void* aligned_alloc(size_t size, size_t alignment) {
  if (size == 0) return nullptr;
#if defined(_WIN32) || defined(_WIN64)
  return _aligned_malloc(size, alignment);
#else
  void* p = nullptr;
  if (posix_memalign(&p, alignment, size) != 0)
    return nullptr;
  return p;
#endif
}

void aligned_free(void* ptr) {
  if (!ptr) return;
#if defined(_WIN32) || defined(_WIN64)
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

}  // namespace util
}  // namespace telehealth
