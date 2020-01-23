#ifndef USTEVENT_NET_DETAIL_DESCRIPTOR_H_
#define USTEVENT_NET_DETAIL_DESCRIPTOR_H_

namespace ustevent
{
namespace net
{
namespace detail
{

#if defined __linux__
using Descriptor = int;
#endif

}
}
}


#endif // USTEVENT_NET_DETAIL_DESCRIPTOR_H_
