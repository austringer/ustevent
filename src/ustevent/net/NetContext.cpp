#include "ustevent/net/NetContext.h"
#include <exception>

#if defined(__linux__)
#include <signal.h>
#include "ustevent/net/detail/linux/EpollSelector.h"
#endif
#include "ustevent/net/MultiplexingContextStrategy.h"

namespace ustevent
{
namespace net
{

NetContext * NetContext::_this_thread_context = nullptr;

NetContext::NetContext()
  : Context()
#if defined(__linux__)
  , _event_selector(::std::make_unique<detail::EpollSelector>(SIGUSR2))
#endif
{
  if (_this_thread_context != nullptr)
  {
    throw ::std::runtime_error("only one context per thread");
  }
  _this_thread_context = this;
}

NetContext::~NetContext() noexcept
{
  _this_thread_context = nullptr;
}

void NetContext::run()
{
  Context::run<MultiplexingContextStrategy>(*this);
}

auto NetContext::selector()
  -> detail::EventSelector &
{
  return *_event_selector;
}

}
}
