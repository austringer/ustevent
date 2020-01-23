#ifndef USTEVENT_CORE_DETAIL_BASICSEMAPHORE_HPP_
#define USTEVENT_CORE_DETAIL_BASICSEMAPHORE_HPP_

#include "ustevent/core/detail/BasicSemaphore.h"
#include <mutex>

namespace ustevent
{
namespace detail
{

template <typename Mutex, typename ConditionVariable>
BasicSemaphore<Mutex, ConditionVariable>::BasicSemaphore() noexcept
  : BasicSemaphore(0)
{}

template <typename Mutex, typename ConditionVariable>
BasicSemaphore<Mutex, ConditionVariable>::BasicSemaphore(int count) noexcept
  : _atomic_count(0)
  , _count(count)
{}

template <typename Mutex, typename ConditionVariable>
void BasicSemaphore<Mutex, ConditionVariable>::notify() noexcept
{
  std::atomic_thread_fence(std::memory_order_release);
  if (_atomic_count.fetch_add(1, std::memory_order_relaxed) < 0)
  {
    {
      ::std::scoped_lock<Mutex> lock(_mutex);
      ++_count;
    }
    _cv.notify_one();
  }
}

template <typename Mutex, typename ConditionVariable>
void BasicSemaphore<Mutex, ConditionVariable>::reset() noexcept
{
  std::atomic_thread_fence(std::memory_order_release);
  if (_atomic_count.exchange(0, std::memory_order_relaxed) < 0)
  {
    {
      ::std::scoped_lock<Mutex> lock(_mutex);
      _count = 0;
    }
    _cv.notify_all();
  }
}

template <typename Mutex, typename ConditionVariable>
void BasicSemaphore<Mutex, ConditionVariable>::wait()
{
  if (_atomic_count.fetch_sub(1, std::memory_order_relaxed) < 1)
  {
    ::std::unique_lock<Mutex> lock(_mutex);
    _cv.wait(lock, [this]() { return _count > 0; });
    --_count;
  }
  std::atomic_thread_fence(std::memory_order_acquire);
}


template <typename Mutex, typename ConditionVariable>
template <typename Clock, typename Duration>
auto BasicSemaphore<Mutex, ConditionVariable>::waitUntil(::std::chrono::time_point<Clock, Duration> const& point)
  -> bool
{
  bool ret = true;
  if (_atomic_count.fetch_sub(1, std::memory_order_relaxed) < 1)
  {
    ::std::unique_lock<Mutex> lock(_mutex);
    ret = _cv.wait_until(lock, point, [this]() { return _count > 0; });
    --_count;
  }
  std::atomic_thread_fence(std::memory_order_acquire);
  return ret;
}

template <typename Mutex, typename ConditionVariable>
template <typename Rep, typename Period>
auto BasicSemaphore<Mutex, ConditionVariable>::waitFor(::std::chrono::duration<Rep, Period> const& duration)
  -> bool
{
  auto now = ::std::chrono::steady_clock::now();
  return waitUntil(now + duration);
}

}
}

#endif // USTEVENT_CORE_DETAIL_BASICSEMAPHORE_HPP_
