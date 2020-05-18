#include "ustevent/net/MultiplexingContextStrategy.h"

#include "ustevent/net/NetContext.h"
#include "ustevent/net/detail/EventSelector.h"

namespace ustevent
{
namespace net
{

MultiplexingContextStrategy::MultiplexingContextStrategy(NetContext & net_context)
  : MultiplexingContextStrategy(net_context, BLOCKING)
{}

MultiplexingContextStrategy::MultiplexingContextStrategy(NetContext & net_context, BlockingFlag blocking_flag)
  : ContextStrategy(net_context)
  , _blocking_flag(blocking_flag)
  , _event_selector(*(net_context._event_selector))
{
  int error;
  ::std::tie(_this_thread, error) = _event_selector.mask();
  if (error)
  {
    // TODO
    throw;
  }
}

MultiplexingContextStrategy::~MultiplexingContextStrategy() noexcept = default;

void MultiplexingContextStrategy::onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  int timeout = 0;
  if (_blocking_flag == POLLING)
  {
    timeout = 0;
  }
  else if (::std::chrono::steady_clock::time_point::max() == time_point)
  {
    timeout = -1;
  }
  else
  {
    auto now = ::std::chrono::steady_clock::now();
    auto duration = ::std::chrono::duration_cast<::std::chrono::milliseconds>(time_point - now);
    timeout = duration.count();
    if (timeout < 1)
    {
      timeout = 1;
    }
  }
  _event_selector.select(timeout);
}

void MultiplexingContextStrategy::onNotified() noexcept
{
  _event_selector.notify(_this_thread);
}

}
}
