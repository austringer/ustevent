#include "ustevent/net/NetContext.h"

#if defined(__linux__)
#include <signal.h>
#include "ustevent/net/detail/linux/EpollSelector.h"
#endif
#include "ustevent/net/MultiplexingContextStrategy.h"

namespace ustevent
{
namespace net
{

NetContext::NetContext()
  : Context()
#if defined(__linux__)
  , _event_selector(::std::make_unique<detail::EpollSelector>(SIGUSR2))
#endif
{}

NetContext::~NetContext() noexcept = default;

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
