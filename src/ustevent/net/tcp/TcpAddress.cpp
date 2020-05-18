#include "ustevent/net/tcp/TcpAddress.h"

#include <cassert>
#include <cstring>
#if defined __linux__
#include <netinet/in.h>
#include <arpa/inet.h>  // inet_ntop
#endif

#include "ustevent/net/ip/IpAddress.h"

namespace ustevent
{
namespace net
{

auto TcpAddress::parse(::std::string_view const host, unsigned short port)
  -> ::std::unique_ptr<TcpAddress>
{
  auto address = TcpIpV4Address::parse(host, port);
  if (address)
  {
    return address;
  }
  else
  {
    return TcpIpV6Address::parse(host, port);
  }
}

auto TcpAddress::parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
  -> bool
{
  if (TcpIpV4Address::parse(host, port, address, address_size))
  {
    return true;
  }
  else
  {
    return TcpIpV6Address::parse(host, port, address, address_size);
  }
}

auto TcpIpV4Address::parse(::std::string_view const host, unsigned short port)
  -> ::std::unique_ptr<TcpIpV4Address>
{
  auto address = ::std::make_unique<TcpIpV4Address>();
  auto tcp_address = reinterpret_cast<sockaddr_in *>(address->data());
  if (IpV4Address::parse(host, &(tcp_address->sin_addr), sizeof(tcp_address->sin_addr)))
  {
    tcp_address->sin_family = AF_INET;
    tcp_address->sin_port = ::htons(port);
    return address;
  }
  return nullptr;
}

auto TcpIpV4Address::parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
  -> bool
{
  if (address_size < SIZE_OF_TCPIPV4_ADDRESS)
  {
    return false;
  }
  auto tcp_address = reinterpret_cast<sockaddr_in *>(address);
  if (IpV4Address::parse(host, &(tcp_address->sin_addr), sizeof(tcp_address->sin_addr)))
  {
    tcp_address->sin_family = AF_INET;
    tcp_address->sin_port = ::htons(port);
    return true;
  }
  return false;
}

TcpIpV4Address::TcpIpV4Address()
{
  static_assert(::std::tuple_size<TcpAddressDataV4>::value >= sizeof(sockaddr_in), "Incompatible structure sockaddr_in.");
}

TcpIpV4Address::TcpIpV4Address(IpV4Address const& ip, unsigned short port)
{
  auto tcp_address = reinterpret_cast<sockaddr_in *>(data());
  tcp_address->sin_family = AF_INET;
  tcp_address->sin_port = ::htons(port);
  ::std::memcpy(&(tcp_address->sin_addr), ip.data(), sizeof(tcp_address->sin_addr));
}

TcpIpV4Address::~TcpIpV4Address() noexcept = default;

auto TcpIpV4Address::protocal() const -> Protocal
{
  return _protocal;
}

TcpIpV4Address::TcpIpV4Address(TcpIpV4Address const& rhs)
{
  ::std::memcpy(data(), rhs.data(), size());
}

auto TcpIpV4Address::operator=(TcpIpV4Address const& rhs)
  -> TcpIpV4Address &
{
  if (this != &rhs)
  {
    ::std::memcpy(data(), rhs.data(), size());
  }
  return *this;
}

auto TcpIpV4Address::data() const
  -> void const*
{
  return _address_v4.data();
}

auto TcpIpV4Address::data()
  -> void *
{
  return _address_v4.data();
}

auto TcpIpV4Address::size() const
  -> ::std::size_t
{
  return _address_v4.size();
}

auto TcpIpV4Address::string() const
  -> ::std::string_view
{
  char str[INET_ADDRSTRLEN + 6];
  auto address = reinterpret_cast<sockaddr_in const*>(data());
  ::inet_ntop(AF_INET, &(address->sin_addr), str, INET_ADDRSTRLEN);
  auto length = ::std::strlen(str);
  ::std::snprintf(str + length, sizeof(str) - length, ":%d", ::ntohs(address->sin_port));
  return { str };
}

auto TcpIpV6Address::parse(::std::string_view const host, unsigned short port)
  -> ::std::unique_ptr<TcpIpV6Address>
{
  auto address = ::std::make_unique<TcpIpV6Address>();
  auto tcp_address = reinterpret_cast<sockaddr_in6 *>(address->data());
  if (IpV6Address::parse(host, &(tcp_address->sin6_addr), sizeof(tcp_address->sin6_addr)))
  {
    tcp_address->sin6_family = AF_INET6;
    tcp_address->sin6_port = ::htons(port);
    return address;
  }
  return nullptr;
}

auto TcpIpV6Address::parse(::std::string_view const host, unsigned short port, void * address, ::std::size_t address_size)
  -> bool
{
  if (address_size < SIZE_OF_TCPIPV6_ADDRESS)
  {
    return false;
  }
  auto tcp_address = reinterpret_cast<sockaddr_in6 *>(address);
  if (IpV6Address::parse(host, &(tcp_address->sin6_addr), sizeof(tcp_address->sin6_addr)))
  {
    tcp_address->sin6_family = AF_INET6;
    tcp_address->sin6_port = ::htons(port);
    return true;
  }
  return false;
}

TcpIpV6Address::TcpIpV6Address()
{
  static_assert(::std::tuple_size<TcpAddressDataV6>::value >= sizeof(sockaddr_in6), "Incompatible structure sockaddr_in6.");
}

TcpIpV6Address::TcpIpV6Address(IpV6Address const& ip, unsigned short port)
{
  auto address = reinterpret_cast<sockaddr_in6 *>(data());
  address->sin6_family = AF_INET6;
  address->sin6_port = ::htons(port);
  ::std::memcpy(&(address->sin6_addr), ip.data(), sizeof(address->sin6_addr));
}

TcpIpV6Address::~TcpIpV6Address() noexcept = default;

TcpIpV6Address::TcpIpV6Address(TcpIpV6Address const& rhs)
{
  ::std::memcpy(data(), rhs.data(), size());
}

auto TcpIpV6Address::operator=(TcpIpV6Address const& rhs)
  -> TcpIpV6Address &
{
  if (this != &rhs)
  {
    ::std::memcpy(data(), rhs.data(), size());
  }
  return *this;
}

auto TcpIpV6Address::protocal() const -> Protocal
{
  return _protocal;
}

auto TcpIpV6Address::data() const
  -> void const*
{
  return _address_v6.data();
}

auto TcpIpV6Address::data()
  -> void *
{
  return _address_v6.data();
}

auto TcpIpV6Address::size() const
  -> ::std::size_t
{
  return _address_v6.size();
}

auto TcpIpV6Address::string() const
  -> ::std::string_view
{
  char str[INET6_ADDRSTRLEN + 8];
  str[0] = '[';
  auto address = reinterpret_cast<sockaddr_in6 const*>(data());
  ::inet_ntop(AF_INET6, &(address->sin6_addr), str + 1, INET6_ADDRSTRLEN);
  auto length = ::std::strlen(str);
  ::std::snprintf(str + length, sizeof(str) - length, "]:%d", ::ntohs(address->sin6_port));
  return { str };
}

}
}
