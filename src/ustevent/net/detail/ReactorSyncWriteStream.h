#ifndef USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_H_
#define USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_H_

#include <string>
#include <memory>
#include <atomic>

namespace ustevent
{
namespace net
{
namespace detail
{

class EventSelector;

template <typename Device>
class EventObject;

template <typename NonBlockingDevice>
class ReactorSyncWriteStream
{
public:
  explicit
  ReactorSyncWriteStream(::std::shared_ptr<EventObject<NonBlockingDevice>> event_device);

  virtual ~ReactorSyncWriteStream() noexcept;

  auto shutdown()
    -> int;

  auto isShutdown() const
    -> bool;

  auto writeSome(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int>;

  auto write(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int>;

  void interrupt();

  void setWriteTimeout(int milliseconds);

  ReactorSyncWriteStream(ReactorSyncWriteStream const&) = delete;
  auto operator=(ReactorSyncWriteStream const&)
    -> ReactorSyncWriteStream & = delete;

protected:
  auto object() const
    -> ::std::shared_ptr<EventObject<NonBlockingDevice>> const&;

  virtual auto nonBlockingWriteSome(void const* data, ::std::size_t size, ::std::size_t * transferred, int * error)
    -> bool = 0;

  virtual auto shutdownStream()
    -> int = 0;

private:
  ::std::shared_ptr<EventObject<NonBlockingDevice>> _event_device;
  ::std::atomic_bool                                _shutdown = false;
  int                                               _read_timeout_milliseconds = -1;
  int                                               _write_timeout_milliseconds = -1;
};

}
}
}

#include "ustevent/net/detail/ReactorSyncWriteStream.hpp"

#endif // USTEVENT_NET_DETAIL_REACTORSYNCWRITESTREAM_H_
