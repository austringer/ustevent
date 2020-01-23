#ifndef USTEVENT_CORE_DETAIL_BASICSEMAPHORE_H_
#define USTEVENT_CORE_DETAIL_BASICSEMAPHORE_H_

#include <atomic>
#include <chrono>

namespace ustevent
{
namespace detail
{

template <typename Mutex, typename ConditionVariable>
class BasicSemaphore
{
public:
  BasicSemaphore() noexcept;

  explicit
  BasicSemaphore(int count) noexcept;

  void notify() noexcept;

  void wait();

  void reset() noexcept;

  template <typename Clock, typename Duration>
  auto waitUntil(::std::chrono::time_point<Clock, Duration> const& point)
    -> bool;

  template <typename Rep, typename Period>
  auto waitFor(::std::chrono::duration<Rep, Period> const& duration)
    -> bool;

  BasicSemaphore(BasicSemaphore const&) = delete;
  auto operator=(BasicSemaphore const&)
    -> BasicSemaphore & = delete;

private:
  std::atomic_int                     _atomic_count;
  int                                 _count;
  mutable Mutex                       _mutex;
  mutable ConditionVariable           _cv;
};

}
}

#include "ustevent/core/detail/BasicSemaphore.hpp"

#endif // USTEVENT_CORE_DETAIL_BASICSEMAPHORE_H_
