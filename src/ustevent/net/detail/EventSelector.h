#ifndef USTEVENT_NET_DETAIL_EVENTSELECTOR_H_
#define USTEVENT_NET_DETAIL_EVENTSELECTOR_H_

#include <thread>
#include <tuple>
#include "ustevent/net/detail/EventNotifier.h"

namespace ustevent
{
namespace net
{
namespace detail
{

class EventSelector
{
public:
  EventSelector() = default;

  virtual ~EventSelector() noexcept = default;

  virtual auto mask()
    -> ::std::tuple<::std::thread::native_handle_type, int> = 0;

  virtual void notify(::std::thread::native_handle_type const& thread_handle) = 0;

  virtual void select(int timeout_milliseconds) = 0;

  virtual auto registerObject(Descriptor descriptor)
    -> ::std::tuple<EventNotifier *, int> = 0;

  virtual void deregisterObject(EventNotifier * event_notifier) noexcept = 0;

  EventSelector(EventSelector const&) = delete;
  auto operator=(EventSelector const&)
    -> EventSelector & = delete;
};

}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTSELECTOR_H_
