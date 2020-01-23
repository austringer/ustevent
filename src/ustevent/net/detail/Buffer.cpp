#include "ustevent/net/detail/Buffer.h"

namespace ustevent
{
namespace net
{
namespace detail
{

Buffer::Buffer()
{}

Buffer::Buffer(void * data, ::std::size_t size)
  : _size(size)
{
  _buffers.emplace_back(data, size);
}

Buffer::~Buffer() noexcept = default;

void Buffer::concat(void * data, ::std::size_t size)
{
  _buffers.emplace_back(data, size);
  _size += size;
}

void Buffer::reset(void * data, ::std::size_t size)
{
  if (_buffers.empty())
  {
    _buffers.emplace_back(data, size);
  }
  else if (_buffers.size() == 1)
  {
    _buffers[0].reset(data, size);
  }
  else
  {
    _buffers.resize(1);
    _buffers[0].reset(data, size);
  }
  _size = size;
}

auto Buffer::address()
  -> ContinuousBuffer::NativeBufferType *
{
  if (!_buffers.empty())
  {
    return reinterpret_cast<ContinuousBuffer::NativeBufferType *>(&_buffers[0]);
  }
  return nullptr;
}

auto Buffer::count() const
  -> ::std::size_t
{
  return _buffers.size();
}

auto Buffer::size() const
  -> ::std::size_t
{
  return _size;
}

void Buffer::clear()
{
  _buffers.clear();
  _size = 0;
}

}
}
}
