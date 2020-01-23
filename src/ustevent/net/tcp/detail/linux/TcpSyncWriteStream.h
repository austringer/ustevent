#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCWRITESTREAM_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCWRITESTREAM_H_

#include "ustevent/net/detail/ReactorSyncWriteStream.h"
#include "ustevent/net/tcp/detail/linux/TcpSocket.h"

namespace ustevent
{
namespace net
{
namespace detail
{

class TcpSyncWriteStream : public ReactorSyncWriteStream<TcpSocket>
{
public:
  explicit
  TcpSyncWriteStream(::std::shared_ptr<EventObject<TcpSocket>> event_driven_socket);

  ~TcpSyncWriteStream() noexcept override;

protected:
  auto nonBlockingWriteSome(void const* data, ::std::size_t size, ::std::size_t * transferred, int * error)
    -> bool override;

  auto shutdownStream()
    -> int override;

  friend class ::ustevent::net::TcpConnection;
};

}
}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCWRITESTREAM_H_
