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

  IPV4 = 0x1,
  IPV6 = 0x2,

  TCP_IPV4 = TCP    | IPV4,
  TCP_IPV6 = TCP    | IPV6,
  UDP_IPV4 = UDP    | IPV4,
  UDP_IPV6 = UDP    | IPV6,
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
