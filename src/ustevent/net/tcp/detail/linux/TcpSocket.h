#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPSOCKET_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPSOCKET_H_

#include <cstddef>
#include <memory>
#include <tuple>
#include "ustevent/net/detail/Descriptor.h"
#include "ustevent/net/Protocal.h"

namespace ustevent
{
namespace net
{

class TcpAddress;
namespace detail
{
class Buffer;

class TcpSocket final
{
public:
  TcpSocket();

  explicit
  TcpSocket(int socket);

  ~TcpSocket() noexcept;

  TcpSocket(TcpSocket && another) noexcept;
  auto operator=(TcpSocket && another) noexcept
    -> TcpSocket &;

  auto descriptor() const
    -> Descriptor;

  auto release()
    -> Descriptor;

  auto open(Protocal protocal)
    -> int;

  auto close()
    -> int;

  enum HowToShutdown
  {
    SHUTDOWN_RD   = 0,
    SHUTDOWN_WR   = 1,
    SHUTDOWN_RDWR = 2,
  };
  auto shutdown(HowToShutdown how)
    -> int;

  auto bind(TcpAddress const& address)
    -> int;

  auto listen()
    -> int;

  auto connect(TcpAddress const& address)
    -> int;

  auto nonBlockingConnect(int * error)
    -> bool;

  auto nonBlockingAccept(int * accepted_socket, ::std::unique_ptr<TcpAddress> * remote_address_ptr, int * error)
    -> bool;

  auto nonBlockingReceive(Buffer & buffer, ::std::size_t * transferred, int * error)
    -> bool;

  auto nonBlockingSend(Buffer & buffer, ::std::size_t * transferred, int * error)
    -> bool;

  auto setReusePort(bool reuse_port)
    -> int;

  auto setNoDelay(bool no_delay)
    -> int;

  TcpSocket(TcpSocket const&) = delete;
  auto operator=(TcpSocket const&)
    -> TcpSocket & = delete;

private:
  static const Descriptor INVALID_FD = -1;

  Descriptor _socket_fd = INVALID_FD;
};

auto getLocalAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>;

auto getRemoteAddress(TcpSocket const& socket)
  -> ::std::tuple<::std::unique_ptr<TcpAddress>, int>;

}
}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPSOCKET_H_
