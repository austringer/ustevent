#ifndef USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_HPP_
#define USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_HPP_

#include "ustevent/net/detail/ReactorSyncWriteStream.h"
#include "ustevent/net/detail/EventSelector.h"
#include "ustevent/net/detail/EventOperation.h"
#include "ustevent/net/Error.h"

namespace ustevent
{
namespace net
{
namespace detail
{

template <typename NonBlockingDevice>
ReactorSyncWriteStream<NonBlockingDevice>::ReactorSyncWriteStream(
  ::std::shared_ptr<EventObject<NonBlockingDevice>> event_device
)
  : _event_device(::std::move(event_device))
{}

template <typename NonBlockingDevice>
ReactorSyncWriteStream<NonBlockingDevice>::~ReactorSyncWriteStream() noexcept = default;

template <typename NonBlockingDevice>
auto ReactorSyncWriteStream<NonBlockingDevice>::shutdown()
  -> int
{
  _shutdown.store(true, ::std::memory_order_release);
  return shutdownStream();
}

template <typename NonBlockingDevice>
auto ReactorSyncWriteStream<NonBlockingDevice>::isShutdown() const
  -> bool
{
  return _shutdown.load(::std::memory_order_acquire);
}

template <typename NonBlockingDevice>
auto ReactorSyncWriteStream<NonBlockingDevice>::writeSome(void const* data, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  EventOperation<NonBlockingDevice> write_operation(_event_device, EVENT_OUT);

  ::std::size_t bytes_written = 0;
  int e = write_operation.perform(
      [this](){ return !this->isShutdown(); },
      [this, data, size, &bytes_written](int * error)
      {
        return this->nonBlockingWriteSome(data, size, &bytes_written, error);
      },
      [&bytes_written](int * error)
      {
        assert(error != nullptr);
        if (*error == EPIPE)
        {
          *error = Error::REMOTELY_CLOSED;
        }
        return true;
      },
      [](int * /* error */){ return false; },
      _write_timeout_milliseconds);
  return { bytes_written, e };
}

template <typename NonBlockingDevice>
auto ReactorSyncWriteStream<NonBlockingDevice>::write(void const* data, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  EventOperation<NonBlockingDevice> write_operation(_event_device, EVENT_OUT);

  ::std::size_t bytes_written = 0;
  unsigned char const* next_data = static_cast<unsigned char const*>(data);
  ::std::size_t next_size = size;
  int e = write_operation.perform(
      [this](){ return !isShutdown(); },
      [this, next_data, next_size, &bytes_written](int * error)
      {
        return nonBlockingWriteSome(next_data, next_size, &bytes_written, error);
      },
      [&bytes_written, &next_data, &next_size](int * error)
      {
        assert(error != nullptr);
        if (*error == EPIPE)
        {
          *error = Error::REMOTELY_CLOSED;
          return true;
        }
        else if (*error)
        {
          return true;
        }
        else if (bytes_written < next_size)
        {
          next_data += bytes_written;
          next_size -= bytes_written;
          return false;
        }
        else if (bytes_written == next_size)
        {
          next_data += bytes_written;
          next_size = 0;
          return true;
        }
        else
        {
          assert(false);
          return true;
        }
      },
      [](int * /* error */){ return false; },
      _write_timeout_milliseconds);
  return { next_data - static_cast<unsigned char const*>(data), e };
}

template <typename NonBlockingDevice>
void ReactorSyncWriteStream<NonBlockingDevice>::interrupt()
{
  EventOperation<NonBlockingDevice> write_operation(_event_device, EVENT_OUT);
  write_operation.interrupt();
}

template <typename NonBlockingDevice>
void ReactorSyncWriteStream<NonBlockingDevice>::setWriteTimeout(int milliseconds)
{
  _write_timeout_milliseconds = milliseconds;
}

template <typename NonBlockingDevice>
auto ReactorSyncWriteStream<NonBlockingDevice>::object() const
  -> ::std::shared_ptr<EventObject<NonBlockingDevice>> const&
{
  return _event_device;
}

}
}
}

#endif // USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_HPP_
