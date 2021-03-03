#ifndef USTEVENT_NET_DETAIL_EVENTOBJECT_H_
#define USTEVENT_NET_DETAIL_EVENTOBJECT_H_

#include "ustevent/net/detail/EventSelector.h"
#include "ustevent/net/detail/EventType.h"

namespace ustevent
{
namespace net
{
namespace detail
{

class EventSelector;
class EventNotifier;

template <typename Resource>
class EventObject
{
public:
  EventObject(EventSelector & event_selector, Resource && opened_device);

  ~EventObject() noexcept;

  auto init()
    -> int;

  auto get()
    -> Resource &;

  auto release()
    -> Resource &&;

  void wait(EventType event);

  template <typename Rep, typename Period>
  auto waitFor(EventType event, std::chrono::duration<Rep, Period> const& time_duration)
    -> bool;

  template <class Clock, class Duration>
  auto waitUntil(EventType event, std::chrono::time_point<Clock, Duration> const& time_point)
    -> bool;

  void notify(EventType event);

private:
  EventSelector &     _event_selector;
  EventNotifier *     _event_notifier = nullptr;

  Resource            _resource;
};

}
}
}

#include "ustevent/net/detail/EventObject.hpp"

#endif // USTEVENT_NET_DETAIL_EVENTOBJECT_H_
