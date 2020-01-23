#ifndef USTEVENT_CORE_DETAIL_BASICBARRIER_HPP_
#define USTEVENT_CORE_DETAIL_BASICBARRIER_HPP_

#include "ustevent/core/detail/BasicBarrier.h"

#include <cassert>

namespace ustevent
{
namespace detail
{

template <typename Mutex, typename ConditionVariable>
BasicBarrier<Mutex, ConditionVariable>::BasicBarrier(::std::size_t count) noexcept
  : _threshold(count)
  , _count(count)
  , _generation(0)
{
  assert(count);
}

template <typename Mutex, typename ConditionVariable>
auto BasicBarrier<Mutex, ConditionVariable>::wait()
  -> bool
{
  ::std::unique_lock<Mutex> lock(_mutex);
  ::std::size_t gen = _generation;

  if (0 == --_count)
  {
    ++_generation;
    _count = _threshold;
    _cv.notify_all();
    return true;
  }

  _cv.wait(lock, [this, gen](){ return gen != _generation; });
  return false;
}

}
}

#endif // USTEVENT_CORE_DETAIL_BASICBARRIER_HPP_
