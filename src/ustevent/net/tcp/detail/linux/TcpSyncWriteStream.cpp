#include "ustevent/net/tcp/detail/linux/TcpSyncWriteStream.h"

#include <cassert>
#include "ustevent/net/detail/Buffer.h"
#include "ustevent/net/tcp/TcpAddress.h"

namespace ustevent
{
namespace net
{
namespace detail
{

TcpSyncWriteStream::~TcpSyncWriteStream() noexcept = default;

TcpSyncWriteStream::TcpSyncWriteStream(::std::shared_ptr<EventObject<TcpSocket>> event_driven_socket)
  : ReactorSyncWriteStream<TcpSocket>(std::move(event_driven_socket))
{}

auto TcpSyncWriteStream::nonBlockingWriteSome(void const* data, ::std::size_t size, ::std::size_t * transferred, int * error)
  -> bool
{
  Buffer b;
  b.reset(const_cast<void *>(data), size);
  assert(object() != nullptr);
  return object()->get().nonBlockingSend(b, transferred, error);
}

auto TcpSyncWriteStream::shutdownStream()
  -> int
{
  assert(object() != nullptr);
  return object()->get().shutdown(TcpSocket::SHUTDOWN_WR);
}

}
}
}
