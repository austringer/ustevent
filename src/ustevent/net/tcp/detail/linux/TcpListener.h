#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPLISTENER_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPLISTENER_H_

#include <atomic>
#include <functional>
#include <memory>
#include "ustevent/net/Listener.h"
#include "ustevent/net/tcp/TcpAddress.h"

namespace ustevent
{
namespace net
{

class NetContext;

namespace detail
{
class EventSelector;

template <typename Device>
class EventObject;

class TcpSocket;
}

class Connection;
class TcpAddress;

class TcpListener : public Listener
{
public:
  static auto open(NetContext & net_context, ::std::unique_ptr<TcpAddress> local_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

  static auto open(NetContext & net_context, ::std::unique_ptr<TcpIpV4Address> local_v4_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

  static auto open(NetContext & net_context, ::std::unique_ptr<TcpIpV6Address> local_v6_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

private:
  TcpListener(
    NetContext & net_context,
    ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_driven_tcp_socket,
    ::std::unique_ptr<TcpIpV4Address> local_v4_address
  );

    TcpListener(
    NetContext & net_context,
    ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_driven_tcp_socket,
    ::std::unique_ptr<TcpIpV6Address> local_v6_address
  );

public:
  ~TcpListener() noexcept;

  void setAcceptTimeout(int milliseconds) override;

  auto accept()
    -> ::std::tuple<::std::unique_ptr<Connection>, int> override;

  auto address()
    -> Address const* override;

  void interrupt();

  void close() override;

  auto getNetContext()
    -> NetContext & override;

  TcpListener(TcpListener const&) = delete;
  auto operator=(TcpListener const&) -> TcpListener & = delete;

private:
  NetContext &                                              _net_context;
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> _event_listen_socket;
  ::std::unique_ptr<TcpAddress>                             _local_address;
  int                                                       _accept_timeout_milliseconds = -1;
  ::std::atomic_bool                                        _is_listening = true;

  auto _isListening() const -> bool;
};

}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPLISTENER_H_
