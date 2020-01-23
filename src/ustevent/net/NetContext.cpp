#include "ustevent/net/NetContext.h"

#if defined(__linux__)
#include <signal.h>
#include "ustevent/net/detail/linux/EpollSelector.h"
#endif

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

auto NetContext::selector()
  -> detail::EventSelector &
{
  return *_event_selector;
}

}
}
