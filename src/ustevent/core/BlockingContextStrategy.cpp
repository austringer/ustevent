#include "ustevent/core/BlockingContextStrategy.h"

namespace ustevent
{

BlockingContextStrategy::BlockingContextStrategy(Context & context)
  : BlockingContextStrategy(context, BLOCKING)
{}

BlockingContextStrategy::BlockingContextStrategy(Context & context, BlockingFlag blocking_flag)
  : ContextStrategy(context)
  , _blocking_flag(blocking_flag)
{}

BlockingContextStrategy::~BlockingContextStrategy() noexcept = default;

void BlockingContextStrategy::onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  if (_blocking_flag == BLOCKING)
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
  if (_blocking_flag == BLOCKING)
  {
    {
      ::std::scoped_lock<thread::Mutex> lock(_suspend_mutex);
      _notified = true;
    }
    _suspend_cv.notify_all();
  }
}

}
