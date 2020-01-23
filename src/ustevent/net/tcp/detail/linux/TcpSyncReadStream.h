#ifndef USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCREADSTREAM_H_
#define USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCREADSTREAM_H_

#include <memory>
#include "ustevent/net/detail/ReactorSyncReadStream.h"
#include "ustevent/net/tcp/detail/linux/TcpSocket.h"

namespace ustevent
{
namespace net
{
namespace detail
{

class TcpSyncReadStream : public ReactorSyncReadStream<TcpSocket>
{
public:
  explicit
  TcpSyncReadStream(::std::shared_ptr<EventObject<TcpSocket>> event_socket);

  ~TcpSyncReadStream() noexcept override;

protected:
  auto nonBlockingReadSome(void * buffer, ::std::size_t size, ::std::size_t * transferred, int * error)
    -> bool override;

  auto shutdownStream()
    -> int override;

  friend class ::ustevent::net::TcpConnection;
};

}
}
}

#endif // USTEVENT_NET_TCP_DETAIL_LINUX_TCPSYNCREADSTREAM_H_
