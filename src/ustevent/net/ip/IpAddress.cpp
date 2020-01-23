#include "ustevent/net/ip/IpAddress.h"
#include <cassert>
#include <cstring>
#include "ustevent/net/Error.h"

#if defined __linux__
#include <arpa/inet.h>
#endif

namespace ustevent
{
namespace net
{

template <>
auto IpAddress::parse<IpAddress::V4>(::std::string_view const host)
  -> ::std::unique_ptr<IpAddress>
{
  auto address = ::std::make_unique<IpAddress>(v4);
  if (::inet_pton(AF_INET, host.data(), address->data()) == 1)
  {
    return address;
  }
  else
  {
    return nullptr;
  }
}

template <>
auto IpAddress::parse<IpAddress::V6>(::std::string_view const host)
  -> ::std::unique_ptr<IpAddress>
{
  auto address = ::std::make_unique<IpAddress>(V6());
  if (::inet_pton(AF_INET6, host.data(), address->data()) == 1)
  {
    return address;
  }
  else
  {
    return nullptr;
  }
}

auto IpAddress::parse(::std::string_view const host)
  -> ::std::unique_ptr<IpAddress>
{
  auto address = parse<V4>(host);
  if (address == nullptr)
  {
    address = parse<V6>(host);
  }
  return address;
}

IpAddress::IpAddress()
  : IpAddress(V6())
{
  static_assert(::std::tuple_size<IPAddressDataV4>::value == sizeof(in_addr), "Incompatible structure in_addr.");
  static_assert(::std::tuple_size<IPAddressDataV6>::value == sizeof(in6_addr), "Incompatible structure in6_addr.");
}

IpAddress::IpAddress(V4)
  : _version(IPv4)
{}

IpAddress::IpAddress(V6)
  : _version(IPv4)
{}

IpAddress::~IpAddress() noexcept = default;

auto IpAddress::protocal() const
  -> Protocal
{
  return _version;
}

auto IpAddress::string() const
  -> ::std::string_view
{
  switch (_version)
  {
  case IPv6:
    {
      char str[INET6_ADDRSTRLEN];
      ::inet_ntop(AF_INET6, &(reinterpret_cast<sockaddr_in6 const*>(&_address_v6[0])->sin6_addr), str, INET6_ADDRSTRLEN);
      return { str };
    }
  case IPv4:
    {
      char str[INET_ADDRSTRLEN];
      ::inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in const*>(&_address_v4[0])->sin_addr), str, INET_ADDRSTRLEN);
      return { str };
    }
  default:
    return {};
  }
}

auto IpAddress::data()
  -> void *
{
  switch (_version)
  {
  case IPv6:
    return &_address_v6[0];
  case IPv4:
    return &_address_v4[0];
  default:
    return nullptr;
  }
}

auto IpAddress::data() const
  -> void const*
{
  switch (_version)
  {
  case IPv6:
    return &_address_v6[0];
  case IPv4:
    return &_address_v4[0];
  default:
    return nullptr;
  }
}

auto IpAddress::size() const
  -> ::std::size_t
{
  switch (_version)
  {
  case IPv6:
    return _address_v6.size();
  case IPv4:
    return _address_v4.size();
  default:
    return 0;
  }
}

auto operator==(IpAddress const& lhs, IpAddress const& rhs)
  -> bool
{
  const void * _address_bytes_v4;
  const void * _address_bytes_v6;
  if (lhs._version == rhs._version)
  {
    return (::std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
  }
  else if (lhs._version == IPv4)
  {
    _address_bytes_v4 = lhs.data();
    _address_bytes_v6 = rhs.data();
  }
  else if (lhs._version == IPv6)
  {
    _address_bytes_v6 = lhs.data();
    _address_bytes_v4 = rhs.data();
  }
  static const uint16_t prefix[] = { 0, 0, 0, 0, 0, 0xFFFF };
  return (::std::memcmp(_address_bytes_v6, prefix, sizeof(prefix)) == 0 &&
          ::std::memcmp(static_cast<uint8_t const*>(_address_bytes_v6) + sizeof(prefix), _address_bytes_v4, sizeof(in_addr)) == 0);
}

}
}
