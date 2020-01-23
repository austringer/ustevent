#ifndef USTEVENT_NET_PROTOCAL_H_
#define USTEVENT_NET_PROTOCAL_H_

namespace ustevent
{
namespace net
{

enum Protocal : unsigned int
{
  UNKNOWN = 0,
  TCP = 0x1 << 8,
  UDP = 0x2 << 8,

  IPv4 = 0x1,
  IPv6 = 0x2,

  TCP_IPv4 = TCP | IPv4,
  TCP_IPv6 = TCP | IPv6,
  UDP_IPv4 = UDP | IPv4,
  UDP_IPv6 = UDP | IPv6,
};

inline
Protocal networkProtocal(Protocal protocal)
{
  return static_cast<Protocal>(protocal & 0xFF);
}

inline
Protocal transportProtocal(Protocal protocal)
{
  return static_cast<Protocal>(protocal & 0xFF00);
}

}
}

#endif // USTEVENT_NET_PROTOCAL_H_
