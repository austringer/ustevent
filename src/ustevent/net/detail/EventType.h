#ifndef USTEVENT_NET_DETAIL_EVENTTYPE_H_
#define USTEVENT_NET_DETAIL_EVENTTYPE_H_

namespace ustevent
{
namespace net
{
namespace detail
{

enum EventType : unsigned int
{
  EVENT_IN    = 0,
  EVENT_OUT   = 1,
  EVENT_TYPES_SIZE,
};

}
}
}

#endif // USTEVENT_NET_DETAIL_EVENTTYPE_H_
