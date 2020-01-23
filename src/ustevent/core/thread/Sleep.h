#ifndef USTEVENT_CORE_THREAD_FUNCTIONS_H_
#define USTEVENT_CORE_THREAD_FUNCTIONS_H_

#include <string>
#include <thread>

namespace ustevent
{

namespace thread
{

constexpr inline auto && yield = ::std::this_thread::yield;

template <typename Clock, typename Duration>
void sleepUntil(std::chrono::time_point<Clock, Duration> const& sleep_time);

template <typename Rep, typename Period>
void sleepFor(std::chrono::duration<Rep, Period> const& sleep_duration);

// Alias templates are never deduced so just wrap the std call

template <typename Clock, typename Duration>
void sleepUntil(std::chrono::time_point<Clock, Duration> const& sleep_time)
{
  ::std::this_thread::sleep_until<Clock, Duration>(sleep_time);
}

template <typename Rep, typename Period>
void sleepFor(std::chrono::duration<Rep, Period> const& sleep_duration)
{
  ::std::this_thread::sleep_for<Rep, Period>(sleep_duration);
}
}
}

#endif // USTEVENT_CORE_THREAD_FUNCTIONS_H_
