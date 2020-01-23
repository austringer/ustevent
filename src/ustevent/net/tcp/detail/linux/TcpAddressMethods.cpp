#include "ustevent/net/tcp/detail/linux/TcpAddressMethods.h"

#include <cassert>
#include <cstring>
#include <sys/socket.h>

#include "ustevent/net/Error.h"
#include "ustevent/net/tcp/TcpAddress.h"
#include "ustevent/net/tcp/detail/linux/TcpSocket.h"

namespace ustevent
{
namespace net
{
namespace detail
{

template <>
auto getLocalAddress<IpAddress::V4>(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  auto local_address = ::std::make_unique<TcpAddress>(IpAddress::v4);

  ::socklen_t address_length = local_address->size();
  errno = 0;
  if (::getsockname(socket.descriptor(), local_address->data(), &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > local_address->size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  auto addr = static_cast<sockaddr *>(local_address->data());
  if (addr->sa_family != AF_INET)
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
  return { ::std::move(local_address), 0 };
}

template <>
auto getLocalAddress<IpAddress::V6>(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  auto local_address = ::std::make_unique<TcpAddress>(IpAddress::v6);

  ::socklen_t address_length = local_address->size();
  errno = 0;
  if (::getsockname(socket.descriptor(), local_address->data(), &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > local_address->size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  auto addr = static_cast<sockaddr *>(local_address->data());
  if (addr->sa_family != AF_INET6)
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
  return { ::std::move(local_address), 0 };
}

template <>
auto getRemoteAddress<IpAddress::V4>(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  auto local_address = ::std::make_unique<TcpAddress>(IpAddress::v4);

  ::socklen_t address_length = local_address->size();
  errno = 0;
  if (::getpeername(socket.descriptor(), local_address->data(), &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > local_address->size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  auto addr = static_cast<sockaddr *>(local_address->data());
  if (addr->sa_family != AF_INET)
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
  return { ::std::move(local_address), 0 };
}

template <>
auto getRemoteAddress<IpAddress::V6>(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  auto local_address = ::std::make_unique<TcpAddress>(IpAddress::v6);

  ::socklen_t address_length = local_address->size();
  errno = 0;
  if (::getpeername(socket.descriptor(), local_address->data(), &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > local_address->size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  auto addr = static_cast<sockaddr *>(local_address->data());
  if (addr->sa_family != AF_INET6)
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
  return { ::std::move(local_address), 0 };
}

}
}
}
