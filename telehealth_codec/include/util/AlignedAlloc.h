#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>

namespace telehealth {
namespace util {

constexpr size_t kDefaultAlignment = 32;  // AVX-friendly

void* aligned_alloc(size_t size, size_t alignment = kDefaultAlignment);
void aligned_free(void* ptr);

template <typename T>
T* aligned_alloc_array(size_t count, size_t alignment = kDefaultAlignment) {
  return static_cast<T*>(aligned_alloc(count * sizeof(T), alignment));
}

struct AlignedDeleter {
  void operator()(void* p) const { aligned_free(p); }
};

}  // namespace util
}  // namespace telehealth
