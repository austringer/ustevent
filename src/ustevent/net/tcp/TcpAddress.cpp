#include "ustevent/net/tcp/TcpAddress.h"

#include <cassert>
#include <cstring>
#if defined __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // inet_ntop
#endif

#include "ustevent/net/ip/IpAddress.h"

namespace ustevent
{
namespace net
{

TcpAddress::TcpAddress()
  : TcpAddress(IpAddress::V6())
{
  static_assert(::std::tuple_size<TCPAddressDataV4>::value == sizeof(sockaddr_in), "Incompatible structure sockaddr_in.");
  static_assert(::std::tuple_size<TCPAddressDataV6>::value == sizeof(sockaddr_in6), "Incompatible structure sockaddr_in6.");
}

TcpAddress::TcpAddress(IpAddress::V4)
  : _protocal(TCP_IPv4)
{
  auto address = reinterpret_cast<sockaddr_in *>(&_address_v4[0]);
  address->sin_family = AF_INET;
}

TcpAddress::TcpAddress(IpAddress::V6)
  : _protocal(TCP_IPv6)
{
  auto address = reinterpret_cast<sockaddr_in6 *>(&_address_v6[0]);
  address->sin6_family = AF_INET6;
}

TcpAddress::TcpAddress(IpAddress const& ip, unsigned short port)
{
  switch (ip.protocal())
  {
  case IPv6:
    {
      _protocal = TCP_IPv6;
      auto address = reinterpret_cast<sockaddr_in6 *>(&_address_v6[0]);
      address->sin6_family = AF_INET6;
      address->sin6_port = ::htons(port);
      address->sin6_flowinfo = 0;
      ::std::memcpy(&(address->sin6_addr), ip.data(), sizeof(address->sin6_addr));
      address->sin6_scope_id = 0;
    }
    break;
  case IPv4:
    {
      _protocal = TCP_IPv4;
      auto address = reinterpret_cast<sockaddr_in *>(&_address_v4[0]);
      address->sin_family = AF_INET;
      address->sin_port = ::htons(port);
      ::std::memcpy(&(address->sin_addr), ip.data(), sizeof(address->sin_addr));
    }
    break;
  default:
    assert(false);
    break;
  }
}

TcpAddress::~TcpAddress() noexcept = default;

auto TcpAddress::protocal() const -> Protocal
{
  return _protocal;
}

auto TcpAddress::string() const
  -> ::std::string_view
{
  switch (_protocal)
  {
  case TCP:
  case TCP_IPv6:
    {
      char str[INET6_ADDRSTRLEN + 8];
      str[0] = '[';
      auto address = reinterpret_cast<sockaddr_in6 const*>(&_address_v6[0]);
      ::inet_ntop(AF_INET6, &(address->sin6_addr), str + 1, INET6_ADDRSTRLEN);
      auto length = ::std::strlen(str);
      ::std::snprintf(str + length, sizeof(str) - length, "]:%d", ::ntohs(address->sin6_port));
      return { str };
    }
  case TCP_IPv4:
    {
      char str[INET_ADDRSTRLEN + 6];
      auto address = reinterpret_cast<sockaddr_in const*>(&_address_v4[0]);
      ::inet_ntop(AF_INET, &(address->sin_addr), str, INET_ADDRSTRLEN);
      auto length = ::std::strlen(str);
      ::std::snprintf(str + length, sizeof(str) - length, ":%d", ::ntohs(address->sin_port));
      return { str };
    }
  default:
    return {};
  }
}

auto TcpAddress::data()
  -> sockaddr *
{
  switch (_protocal)
  {
  case TCP:
  case TCP_IPv6:
    return reinterpret_cast<sockaddr *>(&_address_v6[0]);
  case TCP_IPv4:
    return reinterpret_cast<sockaddr *>(&_address_v4[0]);
  default:
    return nullptr;
  }
}

auto TcpAddress::data() const
  -> sockaddr const*
{
  switch (_protocal)
  {
  case TCP:
  case TCP_IPv6:
    return reinterpret_cast<sockaddr const*>(&_address_v6[0]);
  case TCP_IPv4:
    return reinterpret_cast<sockaddr const*>(&_address_v4[0]);
  default:
    return nullptr;
  }
}

auto TcpAddress::size() const
  -> ::std::size_t
{
  switch (_protocal)
  {
  case TCP:
  case TCP_IPv6:
    return _address_v6.size();
  case TCP_IPv4:
    return _address_v4.size();
  default:
    return 0;
  }
}

}
}
