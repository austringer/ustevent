#include "ustevent/core/BlockingContextStrategy.h"

namespace ustevent
{

BlockingContextStrategy::BlockingContextStrategy(Context & context, bool debugging, bool blocking)
  : ContextStrategy(context, debugging)
  , _blocking(blocking)
{}

BlockingContextStrategy::~BlockingContextStrategy() noexcept = default;

void BlockingContextStrategy::onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  if (_blocking)
  {
    if (time_point == ::std::chrono::steady_clock::time_point::max())
    {
      ::std::unique_lock<thread::Mutex> lock(_suspend_mutex);
      _suspend_cv.wait(lock, [this](){ return _notified; });
      _notified = false;
    }
    else
    {
      ::std::unique_lock<thread::Mutex> lock(_suspend_mutex);
      _suspend_cv.wait_until(lock, time_point, [this](){ return _notified; });
      _notified = false;
    }
  }
}

void BlockingContextStrategy::onNotified() noexcept
{
  if (_blocking)
  {
    {
      ::std::scoped_lock<thread::Mutex> lock(_suspend_mutex);
      _notified = true;
    }
    _suspend_cv.notify_all();
  }
}

}
