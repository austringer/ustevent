#include "ustevent/net/tcp/detail/linux/TcpListener.h"

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

auto TcpListener::open(NetContext & context, TcpAddress const& local_address)
  -> ::std::tuple<::std::unique_ptr<TcpListener>, int>
{
  int error = 0;
  detail::TcpSocket socket;
  if ((error = socket.open()) != 0 ||
      (error = socket.setReusePort(true)) != 0)
  {
    socket.close();
    return { nullptr, error };
  }

  auto event_driven_tcp_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
    context.selector(), ::std::move(socket));
  if ((error = event_driven_tcp_socket->init()) != 0 ||
      (error = event_driven_tcp_socket->get().bind(local_address)) != 0 ||
      (error = event_driven_tcp_socket->get().listen()) != 0)
  {
    return { nullptr, error };
  }
  return {
    ::std::unique_ptr<TcpListener>(new TcpListener(
      context.selector(), ::std::move(event_driven_tcp_socket), local_address
    )), 0 };
}

TcpListener::TcpListener(
  detail::EventSelector & event_selector,
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_driven_tcp_socket,
  TcpAddress const& local_address
  )
  : _event_selector(event_selector)
  , _event_listen_socket(::std::move(event_driven_tcp_socket))
  , _local_address(::std::make_unique<TcpAddress>(local_address))
{
  assert(_event_listen_socket);
}

TcpListener::~TcpListener() noexcept
{
  close();
}

void TcpListener::setAcceptTimeout(int milliseconds)
{
  _accept_timeout_milliseconds = milliseconds;
}

auto TcpListener::accept() -> ::std::tuple<::std::unique_ptr<Connection>, int>
{
  int error = 0;
  ::std::unique_ptr<TcpConnection> connection;

  {
    if (!_isListening())
    {
      error = Error::CLOSED;
      goto finally;
    }

    assert(_local_address != nullptr);
    ::std::unique_ptr<TcpAddress> local_address = ::std::make_unique<TcpAddress>(*_local_address);
    ::std::unique_ptr<TcpAddress> remote_address;
    if ((_local_address->protocal() & IPv6) != 0)
    {
      remote_address = ::std::make_unique<TcpAddress>(IpAddress::v4);
    }
    else if ((_local_address->protocal() & IPv4) != 0)
    {
      remote_address = ::std::make_unique<TcpAddress>(IpAddress::v4);
    }
    else
    {
      error = Error::INVALID_NETWORK_PROTOCAL;
      goto finally;
    }

    detail::Descriptor accepted_socket_fd;

    detail::EventOperation accept_operation(_event_listen_socket, detail::EVENT_IN);

    error = accept_operation.perform(
      [this]() { return this->_isListening(); },
      [this, &accepted_socket_fd, &remote_address](int * error) {
        return _event_listen_socket->get().nonBlockingAccept(&accepted_socket_fd, &remote_address, error);
      },
      [](int * /* error */) { return true; },
      [](int * /* error */) { return false; },
      _accept_timeout_milliseconds);
    if (error)
    {
      goto finally;
    }
    assert(remote_address != nullptr);

    auto event_socket = ::std::make_shared<detail::EventObject<detail::TcpSocket>>(
      _event_selector, detail::TcpSocket(accepted_socket_fd));
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
  return { error ? nullptr : ::std::move(connection), error };
}

auto TcpListener::address() -> Address const*
{
  return _local_address.get();
}

void TcpListener::interrupt()
{
  detail::EventOperation accept_operation(_event_listen_socket, detail::EVENT_IN);
  accept_operation.interrupt();
}

void TcpListener::close()
{
  _is_listening.store(false, ::std::memory_order_release);
  interrupt();
  _event_listen_socket.reset();
  _local_address.reset();
}

auto TcpListener::_isListening() const
  -> bool
{
  return _is_listening.load(::std::memory_order_acquire);
}

}
}
