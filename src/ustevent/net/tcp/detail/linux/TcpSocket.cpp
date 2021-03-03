#include "ustevent/net/tcp/detail/linux/TcpSocket.h"

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h> // close
#include "ustevent/net/detail/Buffer.h"
#include "ustevent/net/tcp/TcpAddress.h"
#include "ustevent/net/Error.h"

namespace ustevent
{
namespace net
{
namespace detail
{

TcpSocket::TcpSocket() = default;

TcpSocket::TcpSocket(int socket)
  : _socket_fd(socket)
{}

TcpSocket::~TcpSocket() noexcept
{
  if (_socket_fd != INVALID_FD)
  {
    close();
  }
}

TcpSocket::TcpSocket(TcpSocket && another) noexcept
{
  _socket_fd = another._socket_fd;
  another._socket_fd = INVALID_FD;
}


auto TcpSocket::operator=(TcpSocket && another) noexcept
  -> TcpSocket &
{
  if (this != &another)
  {
    _socket_fd = another._socket_fd;
    another._socket_fd = INVALID_FD;
  }
  return *this;
}

auto TcpSocket::descriptor() const
  -> Descriptor
{
  return _socket_fd;
}

auto TcpSocket::release()
  -> Descriptor
{
  Descriptor fd = _socket_fd;
  _socket_fd = INVALID_FD;
  return fd;
}

auto TcpSocket::open(Protocal protocal)
  -> int
{
  errno = 0;
  switch (protocal)
  {
  case TCP_IPV4:
    _socket_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    break;
  case TCP_IPV6:
    _socket_fd = ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
    break;
  default:
    if ((protocal & IPV4) == 0 && (protocal & IPV6) == 0)
    {
      return Error::INVALID_NETWORK_PROTOCAL;
    }
    else
    {
      return Error::INVALID_TRANSPORT_PROTOCAL;
    }
  }

  if (_socket_fd == INVALID_FD)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::shutdown(HowToShutdown how)
  -> int
{
  errno = 0;
  if (::shutdown(_socket_fd, static_cast<int>(how)) != 0)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::close()
  -> int
{
  errno = 0;
  if (_socket_fd != INVALID_FD && ::close(_socket_fd) != 0)
  {
    _socket_fd = INVALID_FD;
    return errno;
  }
  return 0;
}

auto TcpSocket::bind(TcpAddress const& tcp_address)
  -> int
{
  int reuse_port = 1;
  ::setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));

  errno = 0;
  if (::bind(_socket_fd, static_cast<sockaddr const*>(tcp_address.data()), tcp_address.size()) != 0)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::listen()
  -> int
{
  errno = 0;
  if (::listen(_socket_fd, 128) != 0)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::connect(TcpAddress const& tcp_address)
  -> int
{
  errno = 0;
  if (::connect(_socket_fd, static_cast<sockaddr const*>(tcp_address.data()), tcp_address.size()) != 0)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::nonBlockingConnect(int * error)
  -> bool
{
  int temp_error = 0;
  if (error == nullptr)
  {
    error = &temp_error;
  }

  ::socklen_t error_length = sizeof(*error);
  errno = 0;
  int ret = ::getsockopt(_socket_fd, SOL_SOCKET, SO_ERROR, error, &error_length);
  if (ret < 0)
  {
    *error = errno;
    return true;
  }
  return (*error != EINPROGRESS);
}

auto TcpSocket::nonBlockingAccept(int * accepted_socket, ::std::unique_ptr<TcpAddress> * remote_address_ptr, int * error)
  -> bool
{
  int temp_new_socket = 0;
  if (accepted_socket == nullptr)
  {
    accepted_socket = &temp_new_socket;
  }
  int temp_error = 0;
  if (error == nullptr)
  {
    error = &temp_error;
  }

  int accepted = INVALID_FD;
  int e = 0;
  while (true)
  {
    errno = 0;
    if (remote_address_ptr && *remote_address_ptr)
    {
      auto & remote_address = *remote_address_ptr;
      ::socklen_t address_length = remote_address->size();
      accepted = ::accept4(_socket_fd, static_cast<sockaddr *>(remote_address->data()), &address_length, SOCK_NONBLOCK);
      if (address_length > remote_address->size())
      {
        remote_address.reset();
      }
    }
    else
    {
      accepted = ::accept4(_socket_fd, nullptr, nullptr, SOCK_NONBLOCK);
    }
    e = errno;

    if (accepted != INVALID_FD)
    {
      *error = 0;
      *accepted_socket = accepted;
      return true;
    }

    // no more stocket events
    if (e == EAGAIN)
    {
      *error = e;
      *accepted_socket = INVALID_FD;
      return false;
    }

    // retry if interrupted by signal
    if (e == EINTR)
    {
      continue;
    }

    *error = e;
    *accepted_socket = INVALID_FD;
    return true;
  }
}

auto TcpSocket::nonBlockingReceive(Buffer & buffer, ::std::size_t * transferred, int * error)
  -> bool
{
  ::std::size_t temp_transferred = 0;
  if (transferred == nullptr)
  {
    transferred = &temp_transferred;
  }
  int temp_error = 0;
  if (error == nullptr)
  {
    error = &temp_error;
  }

  long received = 0;
  int e = 0;
  ::msghdr message_structure = {};
  message_structure.msg_iov = buffer.address();
  message_structure.msg_iovlen = buffer.count();
  while (true)
  {
    errno = 0;
    received = ::recvmsg(_socket_fd, &message_structure, 0);
    if (received > 0)
    {
      *error = 0;
      *transferred = received;
      return true;
    }
    else if (received < 0)
    {
      e = errno;

      // no more stocket events
      if (e == EAGAIN)
      {
        *error = e;
        *transferred = 0;
        return false;
      }

      // retry if interrupted by signal
      if (e == EINTR)
      {
        continue;
      }

      *error = e;
      *transferred = 0;
      return true;
    }
    else
    {
      *transferred = 0;
      return true;
    }
  }
}

auto TcpSocket::nonBlockingSend(Buffer & buffer, ::std::size_t * transferred, int * error)
  -> bool
{
  ::std::size_t temp_transferred = 0;
  if (transferred == nullptr)
  {
    transferred = &temp_transferred;
  }
  int temp_error = 0;
  if (error == nullptr)
  {
    error = &temp_error;
  }

  long sent = 0;
  int e = 0;
  ::msghdr message_structure = {};
  message_structure.msg_iov = buffer.address();
  message_structure.msg_iovlen = buffer.count();
  while (true)
  {
    errno = 0;
    sent = ::sendmsg(_socket_fd, &message_structure, MSG_NOSIGNAL);
    if (sent > 0)
    {
      *error = 0;
      *transferred = sent;
      return true;
    }
    else
    {
      e = errno;

      // no more stocket events
      if (e == EAGAIN)
      {
        *error = e;
        *transferred = 0;
        return false;
      }

      // retry if interrupted by signal
      if (e == EINTR)
      {
        continue;
      }

      *error = e;
      *transferred = 0;
      return true;
    }
  }
}

auto TcpSocket::setReusePort(bool reuse_port)
  -> int
{
  int reuse_port_value = reuse_port ? 1 : 0;
  errno = 0;
  if (::setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port_value, sizeof(reuse_port_value)) != 0)
  {
    return errno;
  }
  return 0;
}

auto TcpSocket::setNoDelay(bool no_delay)
  -> int
{
  int no_delay_value = no_delay ? 1 : 0;
  errno = 0;
  if (::setsockopt(_socket_fd, IPPROTO_TCP, TCP_NODELAY, &no_delay_value, sizeof(no_delay_value)) != 0)
  {
    return errno;
  }
  return 0;
}

auto getLocalAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  ::std::array<::std::uint8_t, SIZE_OF_TCPIP_ADDRESS_STORAGE> address;

  ::socklen_t address_length = address.size();
  errno = 0;
  auto socket_address = static_cast<sockaddr *>(static_cast<void *>(address.data()));
  if (::getsockname(socket.descriptor(), socket_address, &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > address.size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  if (socket_address->sa_family == AF_INET)
  {
    auto tcp_address = ::std::make_unique<TcpIpV4Address>();
    ::std::memcpy(tcp_address->data(), address.data(), tcp_address->size());
    return { ::std::move(tcp_address), 0 };
  }
  else if (socket_address->sa_family == AF_INET6)
  {
    auto tcp_address = ::std::make_unique<TcpIpV6Address>();
    ::std::memcpy(tcp_address->data(), address.data(), tcp_address->size());
    return { ::std::move(tcp_address), 0 };
  }
  else
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
}

auto getRemoteAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>
{
  ::std::array<::std::uint8_t, SIZE_OF_TCPIP_ADDRESS_STORAGE> address;

  ::socklen_t address_length = address.size();
  errno = 0;
  auto socket_address = static_cast<sockaddr *>(static_cast<void *>(address.data()));
  if (::getpeername(socket.descriptor(), socket_address, &address_length) != 0)
  {
    return { nullptr, errno };
  }

  if (address_length > address.size())
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }

  if (socket_address->sa_family == AF_INET)
  {
    auto tcp_address = ::std::make_unique<TcpIpV4Address>();
    ::std::memcpy(tcp_address->data(), address.data(), tcp_address->size());
    return { ::std::move(tcp_address), 0 };
  }
  else if (socket_address->sa_family == AF_INET6)
  {
    auto tcp_address = ::std::make_unique<TcpIpV6Address>();
    ::std::memcpy(tcp_address->data(), address.data(), tcp_address->size());
    return { ::std::move(tcp_address), 0 };
  }
  else
  {
    return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
  }
}

}
}
}
