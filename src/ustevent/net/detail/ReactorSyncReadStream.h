#ifndef USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_H_
#define USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_H_

#include <string>
#include <memory>
#include <atomic>

namespace ustevent
{
namespace net
{
namespace detail
{

template <typename Device>
class EventObject;

template <typename NonBlockingDevice>
class ReactorSyncReadStream
{
public:
  explicit
  ReactorSyncReadStream(::std::shared_ptr<EventObject<NonBlockingDevice>> event_device);

  virtual ~ReactorSyncReadStream() noexcept;

  auto shutdown()
    -> int;

  auto isShutdown() const
    -> bool;

  auto readSome(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int>;

  auto read(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int>;

  void interrupt();

  void setReadTimeout(int milliseconds);

  ReactorSyncReadStream(ReactorSyncReadStream const&) = delete;
  auto operator=(ReactorSyncReadStream const&)
    -> ReactorSyncReadStream & = delete;

protected:
  auto object() const
    -> ::std::shared_ptr<EventObject<NonBlockingDevice>> const&;

  virtual auto nonBlockingReadSome(void * buffer, ::std::size_t size, ::std::size_t * transferred, int * error)
    -> bool = 0;

  virtual auto shutdownStream()
    -> int = 0;

private:
  ::std::shared_ptr<EventObject<NonBlockingDevice>> _event_device;
  ::std::atomic_bool                                _shutdown = false;
  int                                               _read_timeout_milliseconds = -1;
};

}
}
}

#include "ustevent/net/detail/ReactorSyncReadStream.hpp"

#endif // USTEVENT_NET_DETAIL_REACTORSYNCREADSTREAM_H_
