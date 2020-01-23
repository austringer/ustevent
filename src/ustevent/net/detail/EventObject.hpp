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

template <typename Device>
EventObject<Device>::EventObject(EventSelector & event_selector, Device && opened_device)
  : _event_selector(event_selector)
  , _device(::std::move(opened_device))
{}

template <typename Device>
EventObject<Device>::~EventObject() noexcept
{
  if (_event_notifier)
  {
    _device.close();
    _event_selector.deregisterObject(_event_notifier);
    _event_notifier = nullptr;
  }
}

template <typename Device>
auto EventObject<Device>::init()
  -> int
{
  int error;
  ::std::tie(_event_notifier, error) = _event_selector.registerObject(_device.descriptor());
  return error;
}

template <typename Device>
auto EventObject<Device>::get()
  -> Device &
{
  return _device;
}

template <typename Device>
auto EventObject<Device>::release()
  -> Device &&
{
  if (_event_notifier)
  {
    _event_selector.deregisterObject(_event_notifier);
    _event_notifier = nullptr;
  }
  return ::std::move(_device);
}

template <typename Device>
void EventObject<Device>::wait(EventType event)
{
  assert(_event_notifier);
  _event_notifier->_semaphores[event].wait();
}

template <typename Device>
template <typename Rep, typename Period>
auto EventObject<Device>::waitFor(EventType event, std::chrono::duration<Rep, Period> const& time_duration)
  -> bool
{
  assert(_event_notifier);
  return _event_notifier->_semaphores[event].waitFor(time_duration);
}

template <typename Device>
template <class Clock, class Duration>
auto EventObject<Device>::waitUntil(EventType event, std::chrono::time_point<Clock, Duration> const& time_point)
  -> bool
{
  assert(_event_notifier);
  return _event_notifier->_semaphores[event].waitUntil(time_point);
}

template <typename Device>
void EventObject<Device>::notify(EventType event)
{
  assert(_event_notifier);
  _event_notifier->_semaphores[event].notify();
}


}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTOBJECT_HPP_
