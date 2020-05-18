#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPDIALER_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPDIALER_H_

#include <atomic>
#include "ustevent/net/Dialer.h"

namespace ustevent
{
namespace net
{

class NetContext;

namespace detail
{
class EventSelector;

class TcpSocket;

template <typename Device>
class EventObject;
}

class Connection;

class TcpIpV4Address;
class TcpIpV6Address;

class TcpDialer : public Dialer
{
public:
  static auto open(NetContext & context)
    -> ::std::tuple<::std::unique_ptr<TcpDialer>, int>;

private:
  TcpDialer(detail::EventSelector & event_selector);

public:
  ~TcpDialer() noexcept override;

  void setConnectTimeout(int milliseconds) override;

  auto connect(Address const& remote_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int> override;

  auto connect(TcpIpV4Address const& remote_v4_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int>;

  auto connect(TcpIpV6Address const& remote_v6_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int>;

  void interrupt() override;

private:
  detail::EventSelector &                                   _event_selector;
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> _event_dial_socket;
  int                                                       _connect_timeout_milliseconds = -1;
  ::std::atomic_bool                                        _is_connecting = false;

  auto _isConnecting() const -> bool;
};

}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPDIALER_H_
