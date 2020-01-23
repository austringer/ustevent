#ifndef USTEVENT_NET_TCP_TCPADDRESS_H_
#define USTEVENT_NET_TCP_TCPADDRESS_H_

#include <array>
#include <tuple>
#include "ustevent/net/Address.h"
#include "ustevent/net/ip/IpAddress.h"

struct sockaddr;

namespace ustevent
{
namespace net
{
namespace detail
{
class TcpSocket;
}

class TcpAddress : public Address
{
public:
  TcpAddress();

  explicit
  TcpAddress(IpAddress::V4);

  explicit
  TcpAddress(IpAddress::V6);

  TcpAddress(IpAddress const& ip, unsigned short port);

  ~TcpAddress() noexcept override;

  auto protocal() const
    -> Protocal override;

  auto string() const
    -> ::std::string_view override;

  auto data()
    -> sockaddr *;

  auto data() const
    -> sockaddr const*;

  auto size() const
    -> ::std::size_t;

private:
  Protocal _protocal;

  using TCPAddressDataV4 = ::std::array<uint8_t, 16>;
  using TCPAddressDataV6 = ::std::array<uint8_t, 28>;
  union
  {
    TCPAddressDataV4 _address_v4;
    TCPAddressDataV6 _address_v6;
  };
};

}
}

#endif // USTEVENT_NET_TCP_TCPADDRESS_H_
