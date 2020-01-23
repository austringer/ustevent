#ifndef USTEVENT_NET_DETAIL_EVENTOPERATION_HPP_
#define USTEVENT_NET_DETAIL_EVENTOPERATION_HPP_

#include "ustevent/net/detail/EventOperation.h"
#include "ustevent/net/Error.h"
#include "ustevent/net/detail/EventObject.h"

namespace ustevent
{
namespace net
{
namespace detail
{

template <typename Device>
EventOperation<Device>::EventOperation(
    ::std::shared_ptr<EventObject<Device>> const& object,
    ::ustevent::net::detail::EventType binded_event)
  : _binded_event(binded_event)
{
  // ::std::weak_ptr::lock() to guarantee thread safety
  _object = ::std::weak_ptr<EventObject<Device>>(object).lock();
}

template <typename Device>
EventOperation<Device>::~EventOperation() noexcept
{
  _object.reset();
}

template <typename Device>
template <typename Fn1, typename Fn2, typename Fn3, typename Fn4>
int EventOperation<Device>::perform(
    Fn1 const& is_open,
    Fn2 const& non_blocking_operation,
    Fn3 const& on_operated,
    Fn4 const& on_notified,
    int timeout)
{
  if (_object == nullptr)
  {
    return Error::CLOSED;
  }

  _interrupted.store(false, ::std::memory_order_relaxed);
  int error = 0;
  while (is_open())
  {
    if (non_blocking_operation(&error))
    {
      if (on_operated(&error))
      {
        return error;
      }
    }
    else
    {
      // The operation is not finished.
      if (timeout > 0)
      {
        if (!_object->waitFor(_binded_event, ::std::chrono::milliseconds(timeout)))
        {
          return Error::TIMEOUT;
        }
      }
      else
      {
        _object->wait(_binded_event);
      }
      if (_interrupted.load(::std::memory_order_relaxed) == true)
      {
        return Error::INTERRUPTED;
      }

      if (on_notified(&error))
      {
        return error;
      }
    }
  }
  return Error::CLOSED;
}

template <typename Device>
auto EventOperation<Device>::object() const -> ::std::shared_ptr<EventObject<Device>>
{
  return _object;
}

template <typename Device>
void EventOperation<Device>::interrupt()
{
  if (_object)
  {
    _interrupted.store(true, ::std::memory_order_relaxed);
    _object->notify(_binded_event);
  }
}

}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTOPERATION_HPP_
