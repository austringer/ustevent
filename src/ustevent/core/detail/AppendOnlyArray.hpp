#ifndef USTEVENT_CORE_DETAIL_APPENDONLYARRAY_HPP_
#define USTEVENT_CORE_DETAIL_APPENDONLYARRAY_HPP_

#include <stdexcept>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include "ustevent/core/detail/AppendOnlyArray.h"

namespace ustevent
{
namespace detail
{

static const ::std::size_t _bucket_length_data[APPEND_ONLY_ARRAY_BUCKET_SIZE] = {
  0x1,        0x2,        0x4,        0x8,
  0x10,       0x20,       0x40,       0x80,
  0x100,      0x200,      0x400,      0x800,
  0x1000,     0x2000,     0x4000,     0x8000,
  0x10000,    0x20000,    0x40000,    0x80000,
  0x100000,   0x200000,   0x400000,   0x800000,
  0x1000000,  0x2000000,  0x4000000,  0x8000000,
  0x10000000, 0x20000000, 0x40000000, 0x80000000
};
template <typename T>
const ::std::size_t * AppendOnlyArray<T>::_bucket_size = _bucket_length_data;

static const ::std::size_t _bucket_length_sum_data[APPEND_ONLY_ARRAY_BUCKET_SIZE] = {
  0x1,        0x3,        0x7,        0xF,
  0x1F,       0x3F,       0x7F,       0xFF,
  0x1FF,      0x3FF,      0x7FF,      0xFFF,
  0x1FFF,     0x3FFF,     0x7FFF,     0xFFFF,
  0x1FFFF,    0x3FFFF,    0x7FFFF,    0xFFFFF,
  0x1FFFFF,   0x3FFFFF,   0x7FFFFF,   0xFFFFFF,
  0x1FFFFFF,  0x3FFFFFF,  0x7FFFFFF,  0xFFFFFFF,
  0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};
template <typename T>
const ::std::size_t * AppendOnlyArray<T>::_bucket_size_sum = _bucket_length_sum_data;

template <typename T>
AppendOnlyArray<T>::AppendOnlyArray() noexcept
  : _array()
{}

template <typename T>
auto AppendOnlyArray<T>::size() const noexcept
  -> ::std::size_t
{
  return _size.load(::std::memory_order_relaxed);
}

template <typename T>
auto AppendOnlyArray<T>::at(::std::size_t index) const
  -> T const&
{
  ::std::size_t size = _size.load(::std::memory_order_relaxed);
  if (index >= size)
  {
    throw ::std::out_of_range("index: " + ::std::to_string(index) + ", size: " + ::std::to_string(size));
  }

  ::std::size_t index_in_bucket = index;
  ::std::size_t array_index = _arrayIndex(index);
  if (array_index > 0)
  {
    index_in_bucket -= _bucket_size_sum[array_index - 1];
  }
  return _array[array_index][index_in_bucket];
}

template <typename T>
auto AppendOnlyArray<T>::push(T const& element)
  -> ::std::size_t
{
  return emplace(element);
}

template <typename T>
template <typename ... Args>
auto AppendOnlyArray<T>::emplace(Args ... args)
  -> ::std::size_t
{
  ::std::size_t index = _index.fetch_add(1, ::std::memory_order_relaxed);
  ::std::size_t index_in_bucket = index;
  ::std::size_t array_index = _arrayIndex(index);
  if (array_index > 0)
  {
    index_in_bucket -= _bucket_size_sum[array_index - 1];
  }

  T * bucket = _array[array_index].load(::std::memory_order_acquire);
  if (bucket == nullptr)
  {
    T * expected = nullptr;
    bucket = new T[_bucket_size[array_index]];
    if (!_array[array_index].compare_exchange_strong(expected, bucket, ::std::memory_order_acq_rel))
    {
      delete [] bucket;
      bucket = expected;
    }
  }

  bucket[index_in_bucket] = T(::std::forward<Args>(args)...);

  ::std::size_t size = 0;
  do
  {
    size = _size.load(::std::memory_order_acquire);
  }
  while (index != size);
  _size.store(index + 1, ::std::memory_order_release);

  return index;
}

inline
auto _log2(unsigned long long x)
  -> ::std::size_t
{
#if defined(__GNUC__)
  // result undefined if x == 0
  return 8 * sizeof(unsigned long long) - __builtin_clzll(x) - 1;
#elif defined(_MSC_VER)
  return 8 * sizeof(unsigned long long) - __lzcnt64(x) - 1;
#endif
}

template <typename T>
auto AppendOnlyArray<T>::_arrayIndex(::std::size_t index) const noexcept
  -> ::std::size_t
{
  return _log2(index + 1);
}

}
}

#endif // USTEVENT_CORE_DETAIL_APPENDONLYARRAY_HPP_
