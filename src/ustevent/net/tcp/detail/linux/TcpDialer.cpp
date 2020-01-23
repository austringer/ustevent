#include "ustevent/net/tcp/detail/linux/TcpDialer.h"

#include "ustevent/net/Error.h"
#include "ustevent/net/NetContext.h"
#include "ustevent/net/detail/EventObject.h"
#include "ustevent/net/detail/EventOperation.h"
#include "ustevent/net/tcp/TcpAddress.h"
#include "ustevent/net/tcp/TcpConnection.h"
#include "ustevent/net/tcp/detail/linux/TcpSocket.h"
#include "ustevent/net/tcp/detail/linux/TcpAddressMethods.h"
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

auto TcpDialer::connect(Address const& address)
  -> ::std::tuple<::std::unique_ptr<Connection>, int>
{
  int error = 0;
  ::std::unique_ptr<TcpConnection> connection;

  bool expected = false;
  if (_is_connecting.compare_exchange_strong(expected, true, ::std::memory_order_acq_rel))
  {
    detail::TcpSocket socket;
    if ((error = socket.open()) != 0)
    {
      goto finally;
    }

    if (address.protocal() != TCP_IPv6 && address.protocal() != TCP_IPv4)
    {
      error = Error::INVALID_TRANSPORT_PROTOCAL;
      goto finally;
    }
    auto tcp_address = dynamic_cast<TcpAddress const*>(&address);
    assert(tcp_address != nullptr);

    _event_dial_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, ::std::move(socket));
    if ((error = _event_dial_socket->init()) != 0)
    {
      goto finally;
    }

    ::std::unique_ptr<TcpAddress> remote_address = ::std::make_unique<TcpAddress>(*tcp_address);
    ::std::unique_ptr<TcpAddress> local_address;
    // detail::Descriptor connected_socket;
    detail::EventOperation connect_operation(_event_dial_socket, detail::EVENT_OUT);

    error = _event_dial_socket->get().connect(*tcp_address);
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
    if ((remote_address->protocal() & IPv6) != 0)
    {
      ::std::tie(local_address, error) = detail::getLocalAddress<IpAddress::V6>(connected_socket);
    }
    else if ((remote_address->protocal() & IPv4) != 0)
    {
      ::std::tie(local_address, error) = detail::getLocalAddress<IpAddress::V4>(connected_socket);
    }
    else
    {
      assert(false);
    }
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

  _is_connecting.store(false, ::std::memory_order_release);
  _event_dial_socket.reset();

finally:
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
