#ifndef USTEVENT_NET_DETAIL_LINUX_CONTINUOUSBUFFER_H_
#define USTEVENT_NET_DETAIL_LINUX_CONTINUOUSBUFFER_H_

#include <cstddef>
#include <sys/socket.h>

namespace ustevent
{
namespace net
{
namespace detail
{

class ContinuousBuffer
{
public:
  ContinuousBuffer();

  ContinuousBuffer(void * buffer, ::std::size_t length);

  ~ContinuousBuffer() noexcept;

  void reset(void * buffer, ::std::size_t length);

  using NativeBufferType = ::iovec;

private:
  ::iovec _buffer;
};

}
}
}

#endif // USTEVENT_NET_DETAIL_LINUX_CONTINUOUSBUFFER_H_
