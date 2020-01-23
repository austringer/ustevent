#ifndef USTEVENT_NET_DETAIL_BUFFER_H_
#define USTEVENT_NET_DETAIL_BUFFER_H_

#include <vector>

#if defined __linux__
#include "ustevent/net/detail/linux/ContinuousBuffer.h"
#endif

namespace ustevent
{
namespace net
{
namespace detail
{

class Buffer
{
public:
  Buffer();
  Buffer(void * data, ::std::size_t size);
  ~Buffer() noexcept;

  void concat(void * data, ::std::size_t size);
  void reset(void * data, ::std::size_t size);

  auto address()
    -> ContinuousBuffer::NativeBufferType *;
  auto count() const
    -> ::std::size_t;

  auto size() const
    -> ::std::size_t;

  void clear();

private:
  ::std::vector<ContinuousBuffer>   _buffers;
  ::std::size_t                     _size = 0;
};

}
}
}

#endif // USTEVENT_NET_DETAIL_BUFFER_H_
