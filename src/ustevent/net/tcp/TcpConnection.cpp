#include "ustevent/net/tcp/TcpConnection.h"
#include "ustevent/net/Error.h"

#if defined __linux__
#include "ustevent/net/tcp/detail/linux/TcpSyncReadStream.h"
#include "ustevent/net/tcp/detail/linux/TcpSyncWriteStream.h"
#endif

namespace ustevent
{
namespace net
{

TcpConnection::TcpConnection(
  ::std::shared_ptr<detail::EventObject<detail::TcpSocket>> event_socket,
  ::std::unique_ptr<detail::TcpSyncReadStream> read_stream,
  ::std::unique_ptr<detail::TcpSyncWriteStream> write_stream,
  ::std::unique_ptr<TcpAddress> local_address,
  ::std::unique_ptr<TcpAddress> remote_address)
  : _event_socket(::std::move(event_socket))
  , _read_stream(::std::move(read_stream))
  , _write_stream(std::move(write_stream))
  , _local_address(::std::move(local_address))
  , _remote_address(::std::move(remote_address))
{}

TcpConnection::~TcpConnection() noexcept
{}

void TcpConnection::shutdown()
{
  _read_stream->shutdown();
  _write_stream->shutdown();

  interrupt();
}

void TcpConnection::close()
{
  _event_socket->get().close();

  _local_address.reset();
  _remote_address.reset();
}

void TcpConnection::interrupt()
{
  _read_stream->interrupt();
  _write_stream->interrupt();
}

auto TcpConnection::readSome(void * buffer, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  return _read_stream->readSome(buffer, size);
}

auto TcpConnection::writeSome(void const* data, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  return _write_stream->writeSome(data, size);
}

auto TcpConnection::read(void * buffer, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  return _read_stream->read(buffer, size);
}

auto TcpConnection::write(void const* data, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  return _write_stream->write(data, size);
}

auto TcpConnection::localAddress() const
  -> Address const*
{
  return _local_address.get();
}

auto TcpConnection::remoteAddress() const
  -> Address const*
{
  return _remote_address.get();
}

void TcpConnection::setReadTimeout(int milliseconds)
{
  _read_stream->setReadTimeout(milliseconds);
}

void TcpConnection::setWriteTimeout(int milliseconds)
{
  _write_stream->setWriteTimeout(milliseconds);
}

}
}
