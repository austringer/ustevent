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
  static auto open(NetContext & net_context)
    -> ::std::tuple<::std::unique_ptr<TcpDialer>, int>;

private:
  TcpDialer(NetContext & net_context);

public:
  ~TcpDialer() noexcept override;

  void setConnectTimeout(int milliseconds) override;

  auto connect(::std::unique_ptr<Address> remote_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int> override;

  auto connect(::std::unique_ptr<TcpIpV4Address> remote_v4_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int>;

  auto connect(::std::unique_ptr<TcpIpV6Address> remote_v6_address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int>;

  void interrupt() override;

  auto getNetContext()
    -> NetContext & override;

private:
  NetContext &                                              _net_context;
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> _event_dial_socket;
  int                                                       _connect_timeout_milliseconds = -1;
  ::std::atomic_bool                                        _is_connecting = false;

  auto _isConnecting() const -> bool;
};

}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPDIALER_H_
