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
  static auto open(NetContext & context, TcpAddress const& local_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

  static auto open(NetContext & context, TcpIpV4Address const& local_v4_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

  static auto open(NetContext & context, TcpIpV6Address const& local_v6_address)
    -> ::std::tuple<::std::unique_ptr<TcpListener>, int>;

private:
  TcpListener(
    detail::EventSelector & event_selector,
    ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_driven_tcp_socket,
    TcpIpV4Address const& local_v4_address
  );

    TcpListener(
    detail::EventSelector & event_selector,
    ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_driven_tcp_socket,
    TcpIpV6Address const& local_v6_address
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

  TcpListener(TcpListener const&) = delete;
  TcpListener & operator=(TcpListener const&) = delete;

private:
  detail::EventSelector &                                   _event_selector;
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> _event_listen_socket;
  ::std::unique_ptr<TcpAddress>                             _local_address;
  int                                                       _accept_timeout_milliseconds = -1;
  ::std::atomic_bool                                        _is_listening = true;

  auto _isListening() const -> bool;
};

}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPLISTENER_H_
