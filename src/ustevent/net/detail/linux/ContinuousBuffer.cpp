#include "ustevent/net/detail/linux/ContinuousBuffer.h"

namespace ustevent
{
namespace net
{
namespace detail
{

ContinuousBuffer::ContinuousBuffer()
  : _buffer()
{
  static_assert(offsetof(ContinuousBuffer, _buffer) == 0);
  static_assert(sizeof(ContinuousBuffer) == sizeof(::iovec));
}

ContinuousBuffer::ContinuousBuffer(void * buffer, ::std::size_t length)
  : _buffer{ buffer, length }
{}

ContinuousBuffer::~ContinuousBuffer() noexcept = default;

void ContinuousBuffer::reset(void * buffer, ::std::size_t length)
{
  _buffer.iov_base = buffer;
  _buffer.iov_len = length;
}

}
}
}
