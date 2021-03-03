#ifndef USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_HPP_
#define USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_HPP_

#include "ustevent/net/detail/ReactorSyncReadStream.h"
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
ReactorSyncReadStream<NonBlockingDevice>::ReactorSyncReadStream(
  ::std::shared_ptr<EventObject<NonBlockingDevice>> event_device
)
  : _event_device(::std::move(event_device))
{}

template <typename NonBlockingDevice>
ReactorSyncReadStream<NonBlockingDevice>::~ReactorSyncReadStream() noexcept = default;

template <typename NonBlockingDevice>
auto ReactorSyncReadStream<NonBlockingDevice>::shutdown()
  -> int
{
  _shutdown.store(true, ::std::memory_order_release);
  return shutdownStream();
}

template <typename NonBlockingDevice>
auto ReactorSyncReadStream<NonBlockingDevice>::isShutdown() const
  -> bool
{
  return _shutdown.load(::std::memory_order_acquire);
}

template <typename NonBlockingDevice>
auto ReactorSyncReadStream<NonBlockingDevice>::readSome(void * buffer, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  EventOperation<NonBlockingDevice> read_operation(_event_device, EVENT_IN);

  ::std::size_t bytes_read = 0;
  int e = read_operation.perform(
      [this](){ return !isShutdown(); },
      [this, buffer, size, &bytes_read](int * error)
      {
        return nonBlockingReadSome(buffer, size, &bytes_read, error);
      },
      [&bytes_read](int * error)
      {
        assert(error != nullptr);
        if (bytes_read == 0)
        {
          *error = Error::REMOTELY_CLOSED;
        }
        return true;
      },
      [](int * /* error */){ return false; },
      _read_timeout_milliseconds);
  return { bytes_read, e };
}

template <typename NonBlockingDevice>
auto ReactorSyncReadStream<NonBlockingDevice>::read(void * buffer, ::std::size_t size)
  -> ::std::tuple<::std::size_t, int>
{
  EventOperation<NonBlockingDevice> read_operation(_event_device, EVENT_IN);

  ::std::size_t bytes_read = 0;
  unsigned char * next_buffer = static_cast<unsigned char *>(buffer);
  ::std::size_t next_size = size;
  int e = read_operation.perform(
      [this](){ return !isShutdown(); },
      [this, next_buffer, next_size, &bytes_read](int * error)
      {
        return nonBlockingReadSome(next_buffer, next_size, &bytes_read, error);
      },
      [&bytes_read, &next_buffer, &next_size](int * error)
      {
        assert(error != nullptr);
        if (*error)
        {
          return true;
        }
        else if (bytes_read == 0)
        {
          *error = Error::REMOTELY_CLOSED;
          return true;
        }
        else if (bytes_read < next_size)
        {
          next_buffer += bytes_read;
          next_size -= bytes_read;
          return false;
        }
        else if (bytes_read == next_size)
        {
          next_buffer += bytes_read;
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
      _read_timeout_milliseconds);
  return { next_buffer - static_cast<unsigned char *>(buffer), e };
}

template <typename NonBlockingDevice>
void ReactorSyncReadStream<NonBlockingDevice>::interrupt()
{
  EventOperation<NonBlockingDevice> read_operation(_event_device, EVENT_IN);
  read_operation.interrupt();
}

template <typename NonBlockingDevice>
void ReactorSyncReadStream<NonBlockingDevice>::setReadTimeout(int milliseconds)
{
  _read_timeout_milliseconds = milliseconds;
}

template <typename NonBlockingDevice>
auto ReactorSyncReadStream<NonBlockingDevice>::object() const
  -> ::std::shared_ptr<EventObject<NonBlockingDevice>> const&
{
  return _event_device;
}

}
}
}

#endif // USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_HPP_
