#include "ustevent/net/tcp/detail/linux/TcpSyncReadStream.h"

#include <cassert>
#include "ustevent/net/detail/Buffer.h"
#include "ustevent/net/tcp/TcpAddress.h"

namespace ustevent
{
namespace net
{
namespace detail
{

TcpSyncReadStream::~TcpSyncReadStream() noexcept = default;

TcpSyncReadStream::TcpSyncReadStream(::std::shared_ptr<EventObject<TcpSocket>> event_socket)
  : ReactorSyncReadStream<TcpSocket>(std::move(event_socket))
{}

auto TcpSyncReadStream::nonBlockingReadSome(void * buffer, ::std::size_t size, ::std::size_t * transferred, int * error)
  -> bool
{
  Buffer b;
  b.reset(buffer, size);
  assert(object() != nullptr);
  return object()->get().nonBlockingReceive(b, transferred, error);
}

auto TcpSyncReadStream::shutdownStream()
  -> int
{
  assert(object() != nullptr);
  return object()->get().shutdown(TcpSocket::SHUTDOWN_RD);
}


}
}
}
