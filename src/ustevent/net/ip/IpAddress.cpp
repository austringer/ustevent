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

static const uint16_t s_v4mapped_prefix[] = { 0, 0, 0, 0, 0, 0xFFFF };

auto IpAddress::parse(::std::string_view const host)
  -> ::std::unique_ptr<IpAddress>
{
  auto address = IpV4Address::parse(host);
  if (address)
  {
    return address;
  }
  else
  {
    return IpV6Address::parse(host);
  }
}

auto IpAddress::parse(::std::string_view const host, void * address, ::std::size_t address_size)
  -> bool
{
  if (IpV4Address::parse(host, address, address_size))
  {
    return true;
  }
  else
  {
    return IpV6Address::parse(host, address, address_size);
  }
}

auto IpV4Address::parse(::std::string_view const host)
  -> ::std::unique_ptr<IpV4Address>
{
  auto address = ::std::make_unique<IpV4Address>();
  if (::inet_pton(AF_INET, host.data(), address->data()) == 1)
  {
    return address;
  }
  else
  {
    return nullptr;
  }
}

auto IpV4Address::parse(::std::string_view const host, void * address, ::std::size_t address_size)
  -> bool
{
  if (address_size < SIZE_OF_IPV4ADDRESS)
  {
    return false;
  }
  return (::inet_pton(AF_INET, host.data(), address) == 1);
}

IpV4Address::IpV4Address()
{
  static_assert(::std::tuple_size<IpAddressDataV4>::value == sizeof(in_addr), "Incompatible structure in_addr.");
}

IpV4Address::~IpV4Address() noexcept = default;


IpV4Address::IpV4Address(IpV4Address const& rhs)
{
  ::std::memcpy(data(), rhs.data(), size());
}

auto IpV4Address::operator=(IpV4Address const& rhs)
  -> IpV4Address &
{
  if (this != &rhs)
  {
    ::std::memcpy(data(), rhs.data(), size());
  }
  return *this;
}

auto IpV4Address::protocal() const
  -> Protocal
{
  return _protocal;
}

auto IpV4Address::string() const
  -> ::std::string_view
{
  char str[INET_ADDRSTRLEN];
  ::inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in const*>(data())->sin_addr), str, INET_ADDRSTRLEN);
  return { str };
}

auto IpV4Address::data() const
  -> void const*
{
  return _address_v4.data();
}

auto IpV4Address::data()
  -> void *
{
  return _address_v4.data();
}

auto IpV4Address::size() const
  -> ::std::size_t
{
  return _address_v4.size();
}

auto IpV4Address::toV6() const
  -> ::std::unique_ptr<IpV6Address>
{
  auto v6 = ::std::make_unique<IpV6Address>();
  ::std::memcpy(v6->data(), s_v4mapped_prefix, sizeof(s_v4mapped_prefix));
  ::std::memcpy(static_cast<uint8_t *>(v6->data()) + sizeof(s_v4mapped_prefix), data(), size());
  return v6;
}

auto IpV4Address::toV6(void * address, ::std::size_t address_size) const
  -> bool
{
  if (address_size < SIZE_OF_IPV6ADDRESS)
  {
    return false;
  }
  ::std::memcpy(address, s_v4mapped_prefix, sizeof(s_v4mapped_prefix));
  ::std::memcpy(static_cast<uint8_t *>(address) + sizeof(s_v4mapped_prefix), data(), size());
  return true;
}

auto operator==(IpV4Address const& lhs, IpV4Address const& rhs)
  -> bool
{
  return (::std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

auto operator==(IpV4Address const& lhs, IpV6Address const& rhs)
  -> bool
{
  return (::std::memcmp(rhs.data(), s_v4mapped_prefix, sizeof(s_v4mapped_prefix)) == 0 &&
          ::std::memcmp(static_cast<uint8_t const*>(rhs.data()) + sizeof(s_v4mapped_prefix), lhs.data(), lhs.size()) == 0);
}

auto IpV6Address::parse(::std::string_view const host)
  -> ::std::unique_ptr<IpV6Address>
{
  auto address = ::std::make_unique<IpV6Address>();
  if (::inet_pton(AF_INET6, host.data(), address->data()) == 1)
  {
    return address;
  }
  else
  {
    return nullptr;
  }
}

auto IpV6Address::parse(::std::string_view const host, void * address, ::std::size_t address_size)
  -> bool
{
  if (address_size < SIZE_OF_IPV6ADDRESS)
  {
    return false;
  }
  return (::inet_pton(AF_INET6, host.data(), address) == 1);
}

IpV6Address::IpV6Address()
{
  static_assert(::std::tuple_size<IpAddressDataV6>::value == sizeof(in6_addr), "Incompatible structure in6_addr.");
}

IpV6Address::~IpV6Address() noexcept = default;


IpV6Address::IpV6Address(IpV6Address const& rhs)
{
  ::std::memcpy(data(), rhs.data(), size());
}

auto IpV6Address::operator=(IpV6Address const& rhs)
  -> IpV6Address &
{
  if (this != &rhs)
  {
    ::std::memcpy(data(), rhs.data(), size());
  }
  return *this;
}

auto IpV6Address::protocal() const
  -> Protocal
{
  return _protocal;
}

auto IpV6Address::string() const
  -> ::std::string_view
{
  char str[INET6_ADDRSTRLEN];
  ::inet_ntop(AF_INET6, &(reinterpret_cast<sockaddr_in6 const*>(data())->sin6_addr), str, INET6_ADDRSTRLEN);
  return { str };
}

auto IpV6Address::data() const
  -> void const*
{
  return _address_v6.data();
}

auto IpV6Address::data()
  -> void *
{
  return _address_v6.data();
}

auto IpV6Address::size() const
  -> ::std::size_t
{
  return _address_v6.size();
}

auto IpV6Address::toV4() const
  -> ::std::unique_ptr<IpV4Address>
{
  if (::std::memcmp(data(), s_v4mapped_prefix, sizeof(s_v4mapped_prefix)) != 0)
  {
    return nullptr;
  }
  auto v4 = ::std::make_unique<IpV4Address>();
  ::std::memcpy(v4->data(), static_cast<uint8_t const*>(data()) + sizeof(s_v4mapped_prefix), size());
  return v4;
}

auto IpV6Address::toV4(void * address, ::std::size_t address_size) const
  -> bool
{
  if (address_size < SIZE_OF_IPV4ADDRESS)
  {
    return false;
  }
  if (::std::memcmp(data(), s_v4mapped_prefix, sizeof(s_v4mapped_prefix)) != 0)
  {
    return false;
  }
  ::std::memcpy(address, static_cast<uint8_t const*>(data()) + sizeof(s_v4mapped_prefix), size());
  return true;
}

auto operator==(IpV6Address const& lhs, IpV6Address const& rhs)
  -> bool
{
  return (::std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

auto operator==(IpV6Address const& lhs, IpV4Address const& rhs)
  -> bool
{
  return (::std::memcmp(lhs.data(), s_v4mapped_prefix, sizeof(s_v4mapped_prefix)) == 0 &&
          ::std::memcmp(static_cast<uint8_t const*>(lhs.data()) + sizeof(s_v4mapped_prefix), rhs.data(), rhs.size()) == 0);
}

}
}
