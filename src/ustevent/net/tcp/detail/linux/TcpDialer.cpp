#include "ustevent/net/tcp/detail/linux/TcpDialer.h"

#include "ustevent/net/Error.h"
#include "ustevent/net/NetContext.h"
#include "ustevent/net/detail/EventObject.h"
#include "ustevent/net/detail/EventOperation.h"
#include "ustevent/net/tcp/TcpAddress.h"
#include "ustevent/net/tcp/TcpConnection.h"
#include "ustevent/net/tcp/detail/linux/TcpSocket.h"
#include "ustevent/net/tcp/detail/linux/TcpSyncReadStream.h"
#include "ustevent/net/tcp/detail/linux/TcpSyncWriteStream.h"

namespace ustevent
{
namespace net
{

auto TcpDialer::open(NetContext & context)
  -> ::std::tuple<::std::unique_ptr<TcpDialer>, int>
{
  return { ::std::unique_ptr<TcpDialer>(new TcpDialer(context.selector())), 0 };
}

TcpDialer::TcpDialer(detail::EventSelector & event_selector)
  : _event_selector(event_selector)
{}

TcpDialer::~TcpDialer() noexcept
{
}

void TcpDialer::setConnectTimeout(int milliseconds)
{
  _connect_timeout_milliseconds = milliseconds;
}

auto TcpDialer::connect(Address const& remote_address)
  -> ::std::tuple<::std::unique_ptr<Connection>, int>
{
  auto protocal = remote_address.protocal();
  switch (protocal)
  {
  case TCP_IPV4:
    return connect(dynamic_cast<TcpIpV4Address const&>(remote_address));
  case TCP_IPV6:
    return connect(dynamic_cast<TcpIpV6Address const&>(remote_address));
  default:
    if ((protocal & IPV4) == 0 && (protocal & IPV6) == 0)
    {
      return { nullptr, Error::INVALID_NETWORK_PROTOCAL };
    }
    else
    {
      return { nullptr, Error::INVALID_TRANSPORT_PROTOCAL };
    }
  }
}

auto TcpDialer::connect(TcpIpV4Address const& remote_v4_address)
  -> ::std::tuple<::std::unique_ptr<Connection>, int>
{
  int error = 0;
  ::std::unique_ptr<TcpConnection> connection;

  bool expected = false;
  if (_is_connecting.compare_exchange_strong(expected, true, ::std::memory_order_acq_rel))
  {
    detail::TcpSocket socket;
    if ((error = socket.open(TCP_IPV4)) != 0)
    {
      goto finally;
    }

    _event_dial_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, ::std::move(socket));
    if ((error = _event_dial_socket->init()) != 0)
    {
      goto finally;
    }

    // detail::Descriptor connected_socket;
    detail::EventOperation connect_operation(_event_dial_socket, detail::EVENT_OUT);

    error = _event_dial_socket->get().connect(remote_v4_address);
    if (error == EINPROGRESS)
    {
      error = connect_operation.perform(
        [this]() { return true; },
        [this](int * error) {
          return _event_dial_socket->get().nonBlockingConnect(error);
        },
        [](int * /* error */) { return true; },
        [](int * /* error */) { return false; },
        _connect_timeout_milliseconds);
      if (error)
      {
        goto finally;
      }
    }
    else if (error != 0)
    {
      goto finally;
    }

    auto connected_socket = _event_dial_socket->release();

    ::std::unique_ptr<TcpAddress> remote_address = ::std::make_unique<TcpIpV4Address>(remote_v4_address);
    ::std::unique_ptr<TcpAddress> local_address;
    ::std::tie(local_address, error) = detail::getLocalAddress(connected_socket);
    if (error)
    {
      goto finally;
    }

    auto event_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, ::std::move(connected_socket));
    if ((error = event_socket->init()) != 0)
    {
      goto finally;
    }

    auto read_stream = ::std::make_unique<detail::TcpSyncReadStream>(event_socket);
    auto write_stream = ::std::make_unique<detail::TcpSyncWriteStream>(event_socket);

    connection = ::std::unique_ptr<TcpConnection>(new TcpConnection(  // private constructor
      ::std::move(event_socket),
      ::std::move(read_stream),
      ::std::move(write_stream),
      ::std::move(local_address),
      ::std::move(remote_address)));
  }

finally:
  _event_dial_socket.reset();
  _is_connecting.store(false, ::std::memory_order_release);
  return { error ? nullptr : ::std::move(connection), error };
}

auto TcpDialer::connect(TcpIpV6Address const& remote_v6_address)
  -> ::std::tuple<::std::unique_ptr<Connection>, int>
{
  int error = 0;
  ::std::unique_ptr<TcpConnection> connection;

  bool expected = false;
  if (_is_connecting.compare_exchange_strong(expected, true, ::std::memory_order_acq_rel))
  {

    detail::TcpSocket socket;
    if ((error = socket.open(TCP_IPV6)) != 0)
    {
      goto finally;
    }

    _event_dial_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, ::std::move(socket));
    if ((error = _event_dial_socket->init()) != 0)
    {
      goto finally;
    }

    // detail::Descriptor connected_socket;
    detail::EventOperation connect_operation(_event_dial_socket, detail::EVENT_OUT);

    error = _event_dial_socket->get().connect(remote_v6_address);
    if (error == EINPROGRESS)
    {
      error = connect_operation.perform(
        [this]() { return true; },
        [this](int * error) {
          return _event_dial_socket->get().nonBlockingConnect(error);
        },
        [](int * /* error */) { return true; },
        [](int * /* error */) { return false; },
        _connect_timeout_milliseconds);
      if (error)
      {
        goto finally;
      }
    }
    else if (error != 0)
    {
      goto finally;
    }

    auto connected_socket = _event_dial_socket->release();

    ::std::unique_ptr<TcpAddress> remote_address = ::std::make_unique<TcpIpV6Address>(remote_v6_address);
    ::std::unique_ptr<TcpAddress> local_address;
    ::std::tie(local_address, error) = detail::getLocalAddress(connected_socket);
    if (error)
    {
      goto finally;
    }
    assert(local_address->protocal() == TCP_IPV6);

    auto event_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, ::std::move(connected_socket));
    if ((error = event_socket->init()) != 0)
    {
      goto finally;
    }

    auto read_stream = ::std::make_unique<detail::TcpSyncReadStream>(event_socket);
    auto write_stream = ::std::make_unique<detail::TcpSyncWriteStream>(event_socket);

    connection = ::std::unique_ptr<TcpConnection>(new TcpConnection(  // private constructor
      ::std::move(event_socket),
      ::std::move(read_stream),
      ::std::move(write_stream),
      ::std::move(local_address),
      ::std::move(remote_address)));
  }

finally:
  _event_dial_socket.reset();
  _is_connecting.store(false, ::std::memory_order_release);
  return { error ? nullptr : ::std::move(connection), error };
}

void TcpDialer::interrupt()
{
  if (_isConnecting())
  {
    detail::EventOperation connect_operation(_event_dial_socket, detail::EVENT_OUT);
    connect_operation.interrupt();
  }
}

auto TcpDialer::_isConnecting() const
  -> bool
{
  return _is_connecting.load(::std::memory_order_acquire);
}

}
}
