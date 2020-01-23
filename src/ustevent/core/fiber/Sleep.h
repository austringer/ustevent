#ifndef USTEVENT_CORE_FIBER_SLEEP_H_
#define USTEVENT_CORE_FIBER_SLEEP_H_

#include <string>
#include "boost/fiber/operations.hpp"

namespace ustevent
{
namespace fiber
{

constexpr inline auto && yield = ::boost::this_fiber::yield;

template <typename Clock, typename Duration>
void sleepUntil(std::chrono::time_point<Clock, Duration> const& sleep_time);

template <typename Rep, typename Period>
void sleepFor(std::chrono::duration<Rep, Period> const& sleep_duration);

// Alias templates are never deduced so just wrap the boost call

template <typename Clock, typename Duration>
void sleepUntil(std::chrono::time_point<Clock, Duration> const& sleep_time)
{
  ::boost::this_fiber::sleep_until<Clock, Duration>(sleep_time);
}

template <typename Rep, typename Period>
void sleepFor(std::chrono::duration<Rep, Period> const& sleep_duration)
{
  ::boost::this_fiber::sleep_for<Rep, Period>(sleep_duration);
}

}
}

#endif // USTEVENT_CORE_FIBER_SLEEP_H_
