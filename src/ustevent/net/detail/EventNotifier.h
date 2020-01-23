#ifndef USTEVENT_NET_DETAIL_EVENTNOTIFIER_H_
#define USTEVENT_NET_DETAIL_EVENTNOTIFIER_H_

#include "ustevent/core/fiber/Semaphore.h"
#include "ustevent/net/detail/Descriptor.h"
#include "ustevent/net/detail/EventType.h"

namespace ustevent
{
namespace net
{
namespace detail
{

struct EventNotifier
{
  Descriptor              _descriptor;
  fiber::Semaphore        _semaphores[EVENT_TYPES_SIZE];
};

}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTNOTIFIER_H_
