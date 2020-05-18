#ifndef USTEVENT_NET_TCP_TCPADDRESS_H_
#define USTEVENT_NET_TCP_TCPADDRESS_H_

#include <array>
#include <tuple>
#if defined __linux__
#include <sys/socket.h>
#endif

#include "ustevent/net/Address.h"
#include "ustevent/net/ip/IpAddress.h"

namespace ustevent
{
namespace net
{
namespace detail
{
class TcpSocket;
}

enum
{
  SIZE_OF_TCPIPV4_ADDRESS = 16,
  SIZE_OF_TCPIPV6_ADDRESS = 28,
  SIZE_OF_TCPIP_ADDRESS_STORAGE = sizeof(sockaddr_storage),
};

class TcpAddress : public Address
{
public:
  static auto parse(::std::string_view const host, unsigned short port)
    -> ::std::unique_ptr<TcpAddress>;

  static auto parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
    -> bool;
};

class TcpIpV4Address : public TcpAddress
{
public:
  static auto parse(::std::string_view const host, unsigned short port)
    -> ::std::unique_ptr<TcpIpV4Address>;

  static auto parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
    -> bool;

  TcpIpV4Address();

  TcpIpV4Address(IpV4Address const& address, unsigned short port);

  ~TcpIpV4Address() noexcept override;

  TcpIpV4Address(TcpIpV4Address const& rhs);

  auto operator=(TcpIpV4Address const& rhs)
    -> TcpIpV4Address &;

  auto protocal() const
    -> Protocal override;

  auto data() const
    -> void const* override;

  auto data()
    -> void * override;

  auto size() const
    -> ::std::size_t override;

  auto string() const
    -> ::std::string_view override;

private:
  Protocal                  _protocal = TCP_IPV4;

  using TcpAddressDataV4 = ::std::array<uint8_t, SIZE_OF_TCPIPV4_ADDRESS>;
  TcpAddressDataV4          _address_v4;
};

class TcpIpV6Address : public TcpAddress
{
public:
  static auto parse(::std::string_view const host, unsigned short port)
    -> ::std::unique_ptr<TcpIpV6Address>;

  static auto parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
    -> bool;

  TcpIpV6Address();

  TcpIpV6Address(IpV6Address const& address, unsigned short port);

  ~TcpIpV6Address() noexcept override;

  TcpIpV6Address(TcpIpV6Address const& rhs);

  auto operator=(TcpIpV6Address const& rhs)
    -> TcpIpV6Address &;

  auto protocal() const
    -> Protocal override;

  auto data() const
    -> void const* override;

  auto data()
    -> void * override;

  auto size() const
    -> ::std::size_t override;

  auto string() const
    -> ::std::string_view override;

private:
  Protocal                  _protocal = TCP_IPV6;

  using TcpAddressDataV6 = ::std::array<uint8_t, SIZE_OF_TCPIPV6_ADDRESS>;
  TcpAddressDataV6          _address_v6;
};

}
}

#endif // USTEVENT_NET_TCP_TCPADDRESS_H_
