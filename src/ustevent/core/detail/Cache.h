#ifndef USTEVENT_CORE_DETAIL_CACHE_H_
#define USTEVENT_CORE_DETAIL_CACHE_H_

#include <cstdint>

namespace ustevent
{

class Cache
{
public:
  static constexpr unsigned int LINE_SIZE = 64;
  static constexpr unsigned int PREFETCH_STRIDE = LINE_SIZE;  // SSE prefetch at least 32 bytes

  static void prefetchForReading(void const* address) noexcept;
  static void prefetchForWriting(void * address) noexcept;

  static void prefetchForReading(void const* address, ::std::size_t length) noexcept;
  static void prefetchForWriting(void * address, ::std::size_t length) noexcept;
};

inline
void Cache::prefetchForReading(void const* address) noexcept
{
  // L1 cache : hint == 1
  __builtin_prefetch(address, 0, 1);
}

inline
void Cache::prefetchForReading(void const* address, ::std::size_t length) noexcept
{
  char const* pointer = static_cast<char const*>(address);
  char const* end = pointer + static_cast<::std::uintptr_t>(length);
  while (pointer < end)
  {
    prefetchForReading(pointer);
    pointer += static_cast<::std::uintptr_t>(PREFETCH_STRIDE);
  }
}

inline
void Cache::prefetchForWriting(void * address) noexcept
{
  // L1 cache : hint == 1
  __builtin_prefetch(address, 1, 1);
}

inline
void Cache::prefetchForWriting(void * address, ::std::size_t length) noexcept
{
  char * pointer = static_cast<char *>(address);
  char * end = pointer + static_cast<::std::uintptr_t>(length);
  while (pointer < end)
  {
    prefetchForWriting(pointer);
    pointer += static_cast<::std::uintptr_t>(PREFETCH_STRIDE);
  }
}

}

#define USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING alignas(::ustevent::Cache::LINE_SIZE)

#endif // USTEVENT_CORE_DETAIL_CACHE_H_
