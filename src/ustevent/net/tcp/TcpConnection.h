#ifndef USTEVENT_NET_TCP_TCPCONNECTION_H_
#define USTEVENT_NET_TCP_TCPCONNECTION_H_

#include <memory>
#include "ustevent/net/Connection.h"
#include "ustevent/net/detail/EventType.h"
#include "ustevent/net/tcp/TcpAddress.h"

namespace ustevent
{
namespace net
{
namespace detail
{

template <typename Device>
class EventObject;
class TcpSocket;
class TcpSyncReadStream;
class TcpSyncWriteStream;
class TcpDialer;
}

class TcpConnection : public Connection
{
private:
  TcpConnection(
    ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_socket,
    ::std::unique_ptr<detail::TcpSyncReadStream> read_stream,
    ::std::unique_ptr<detail::TcpSyncWriteStream> write_stream,
    ::std::unique_ptr<TcpAddress> local_address,
    ::std::unique_ptr<TcpAddress> remote_address);

public:
  ~TcpConnection() noexcept override;

  void shutdown();

  void close() override;

  void interrupt() override;

  // return: data, bytes received, error
  auto readSome(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> override;

  // return: bytes sent, error
  auto writeSome(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> override;

    // return: data, bytes received, error
  auto read(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> override;

  // return: bytes sent, error
  auto write(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> override;

  auto localAddress() const
    -> Address const* override;

  auto remoteAddress() const
    -> Address const* override;

  void setReadTimeout(int milliseconds) override;

  void setWriteTimeout(int milliseconds) override;

private:
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> _event_socket;
  ::std::unique_ptr<detail::TcpSyncReadStream>              _read_stream;
  ::std::unique_ptr<detail::TcpSyncWriteStream>             _write_stream;

  ::std::unique_ptr<TcpAddress>                             _local_address;
  ::std::unique_ptr<TcpAddress>                             _remote_address;

  friend class TcpListener;
  friend class TcpDialer;
};

}
}

#endif // USTEVENT_NET_TCP_TCPCONNECTION_H_
