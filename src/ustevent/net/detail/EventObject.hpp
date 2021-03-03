#ifndef USTEVENT_NET_DETAIL_EVENTOBJECT_HPP_
#define USTEVENT_NET_DETAIL_EVENTOBJECT_HPP_

#include <cassert>
#include "ustevent/net/detail/EventObject.h"
#include "ustevent/net/detail/EventNotifier.h"

namespace ustevent
{
namespace net
{
namespace detail
{

template <typename Resource>
EventObject<Resource>::EventObject(EventSelector & event_selector, Resource && opened_device)
  : _event_selector(event_selector)
  , _resource(::std::move(opened_device))
{}

template <typename Resource>
EventObject<Resource>::~EventObject() noexcept
{
  if (_event_notifier)
  {
    _resource.close();
    _event_selector.deregisterObject(_event_notifier);
    _event_notifier = nullptr;
  }
}

template <typename Resource>
auto EventObject<Resource>::init()
  -> int
{
  int error;
  ::std::tie(_event_notifier, error) = _event_selector.registerObject(_resource.descriptor());
  return error;
}

template <typename Resource>
auto EventObject<Resource>::get()
  -> Resource &
{
  return _resource;
}

template <typename Resource>
auto EventObject<Resource>::release()
  -> Resource &&
{
  if (_event_notifier)
  {
    _event_selector.deregisterObject(_event_notifier);
    _event_notifier = nullptr;
  }
  return ::std::move(_resource);
}

template <typename Resource>
void EventObject<Resource>::wait(EventType event)
{
  assert(_event_notifier);
  _event_notifier->_semaphores[event].wait();
}

template <typename Resource>
template <typename Rep, typename Period>
auto EventObject<Resource>::waitFor(EventType event, std::chrono::duration<Rep, Period> const& time_duration)
  -> bool
{
  assert(_event_notifier);
  return _event_notifier->_semaphores[event].waitFor(time_duration);
}

template <typename Resource>
template <class Clock, class Duration>
auto EventObject<Resource>::waitUntil(EventType event, std::chrono::time_point<Clock, Duration> const& time_point)
  -> bool
{
  assert(_event_notifier);
  return _event_notifier->_semaphores[event].waitUntil(time_point);
}

template <typename Resource>
void EventObject<Resource>::notify(EventType event)
{
  assert(_event_notifier);
  _event_notifier->_semaphores[event].notify();
}


}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTOBJECT_HPP_
