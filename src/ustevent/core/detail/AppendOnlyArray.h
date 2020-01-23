#ifndef USTEVENT_CORE_DETAIL_APPENDONLYARRAY_H_
#define USTEVENT_CORE_DETAIL_APPENDONLYARRAY_H_

#include <atomic>
#include "ustevent/core/detail/Cache.h"

namespace ustevent
{
namespace detail
{

constexpr ::std::size_t APPEND_ONLY_ARRAY_BUCKET_SIZE = 32; // hold at most (2^33 -1) elements

template <typename T>
class AppendOnlyArray
{
public:
  AppendOnlyArray() noexcept;

  auto size() const noexcept
    -> ::std::size_t;

  auto at(::std::size_t index) const
    -> T const&;

  auto push(T const& element)
    -> ::std::size_t;

  template <typename ... Args>
  auto emplace(Args ... args)
    -> ::std::size_t;

  AppendOnlyArray(AppendOnlyArray const&) = delete;
  auto operator=(AppendOnlyArray const&)
    -> AppendOnlyArray & = delete;

private:

  auto _arrayIndex(::std::size_t count) const noexcept
    -> ::std::size_t;

  static const ::std::size_t *    _bucket_size;
  static const ::std::size_t *    _bucket_size_sum;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic_size_t            _index = 0;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic_size_t            _size = 0;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic<T *>              _array[APPEND_ONLY_ARRAY_BUCKET_SIZE];
};

}
}

#include "ustevent/core/detail/AppendOnlyArray.hpp"

#endif // USTEVENT_CORE_DETAIL_APPENDONLYARRAY_H_
