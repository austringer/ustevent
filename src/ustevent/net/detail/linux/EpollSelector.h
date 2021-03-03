#ifndef USTEVENT_NET_DETAIL_LINUX_EPOLLSELECTOR_H_
#define USTEVENT_NET_DETAIL_LINUX_EPOLLSELECTOR_H_

#include <chrono>
#include "ustevent/core/concurrency/ObjectPool.h"
#include "ustevent/net/detail/EventSelector.h"

namespace ustevent
{
namespace net
{
namespace detail
{

class EpollSelector : public EventSelector
{
public:
  explicit
  EpollSelector(::std::uint32_t signal);

  ~EpollSelector() noexcept override;

  auto mask()
    -> ::std::tuple<::std::thread::native_handle_type, int> override;

  void notify(::std::thread::native_handle_type const& thread_handle) override;

  void select(int timeout_milliseconds) override;

  auto registerObject(Descriptor descriptor)
    -> ::std::tuple<EventNotifier *, int> override;

  void deregisterObject(EventNotifier * event_notifier) noexcept override;

private:
  static const int    INVALID_FD = -1;

  int                 _epoll_fd = INVALID_FD;

  ::std::uint32_t     _signal = 0;
  int                 _signal_fd = INVALID_FD;

  ::ustevent::ObjectPool<EventNotifier> _notifier_pool;

  auto _initEpollFd()
    -> int;

  void _destoryEpollFd() noexcept;

  auto _initSignalFd(::std::uint32_t notification_signal)
    -> int;

  void _destroySignalFd() noexcept;

  auto _readSignal()
    -> ::std::uint32_t;

  auto _allocateNotifier()
    -> EventNotifier *;

  void _freeNotifier(EventNotifier * event_notifier);
};

}
}
}

#endif // USTEVENT_NET_DETAIL_LINUX_EPOLLSELECTOR_H_
