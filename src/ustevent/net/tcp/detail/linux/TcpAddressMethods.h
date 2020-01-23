#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPADDRESSMETHODS_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPADDRESSMETHODS_H_

#include <memory>
#include <tuple>
#include "ustevent/net/detail/Descriptor.h"
#include "ustevent/net/ip/IpAddress.h"

namespace ustevent
{
namespace net
{
class TcpAddress;

namespace detail
{

class TcpSocket;

template <typename IPVersion>
auto getLocalAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>;

template <typename IPVersion>
auto getRemoteAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>;

}
}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPADDRESSMETHODS_H_
