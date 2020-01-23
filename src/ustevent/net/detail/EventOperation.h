#ifndef USTEVENT_NET_DETAIL_EVENTOPERATION_H_
#define USTEVENT_NET_DETAIL_EVENTOPERATION_H_

#include <atomic>
#include <memory>
#include "ustevent/net/detail/EventType.h"
#include "ustevent/net/detail/EventObject.h"

namespace ustevent
{
namespace net
{
namespace detail
{

// Provide blocking-like operation. The caller fiber will be yield out until the I/O operation is finished.
template <typename Device>
class EventOperation
{
public:
  EventOperation(
    ::std::shared_ptr<EventObject<Device>> const& object,
    EventType binded_event);

  ~EventOperation() noexcept;

  template <typename Fn1, typename Fn2, typename Fn3, typename Fn4>
  auto perform(
    Fn1 const& is_open,
    Fn2 const& non_blocking_operation,
    Fn3 const& on_operated,
    Fn4 const& on_notified,
    int timeout)
    -> int;

  auto object() const
    -> ::std::shared_ptr<EventObject<Device>>;

  void interrupt();

  EventOperation(EventOperation const&) = delete;
  auto operator=(EventOperation const&)
    -> EventOperation & = delete;

private:
  ::std::shared_ptr<EventObject<Device>>    _object;

  EventType                                 _binded_event;
  ::std::atomic_bool                        _interrupted = false;
};

}
}
}

#include "ustevent/net/detail/EventOperation.hpp"

#endif // USTEVENT_NET_DETAIL_EVENTOPERATION_H_
