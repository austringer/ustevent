#include "ustevent/net/detail/linux/EpollSelector.h"

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/signalfd.h>

namespace ustevent
{
namespace net
{
namespace detail
{

EpollSelector::EpollSelector(::std::uint32_t signal)
{
  if (_initEpollFd() > 0 ||
      _initSignalFd(signal) > 0)
  {
    throw;
  }
}

EpollSelector::~EpollSelector() noexcept
{
  _destroySignalFd();
  _destoryEpollFd();
}

auto EpollSelector::mask()
  -> ::std::tuple<::std::thread::native_handle_type, int>
{
  ::sigset_t mask;
  ::sigemptyset(&mask);
  ::sigaddset(&mask, _signal);
  return { ::pthread_self(), ::pthread_sigmask(SIG_BLOCK, &mask, nullptr) };
}

void EpollSelector::notify(::std::thread::native_handle_type const& thread_handle)
{
  ::pthread_kill(thread_handle, _signal);
}

void EpollSelector::select(int timeout_milliseconds)
{
  static const int EPOLL_EVENTS_MAX = 128;

  ::epoll_event events[EPOLL_EVENTS_MAX];
  errno = 0;
  int event_count = ::epoll_wait(_epoll_fd, events, EPOLL_EVENTS_MAX, timeout_milliseconds);
  for (int i = 0; i < event_count; ++i)
  {
    if (events[i].data.fd == _signal_fd)
    {
      _readSignal();
      continue;
    }

    EventNotifier * event_notifier = static_cast<EventNotifier *>(events[i].data.ptr);
    if (event_notifier)
    {
      // EventNotifier::notify() will be returned immediately.
      // The descriptor events will be scheduled and handled by the thread that calling PollerContext.run().
      if (events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP))
      {
        event_notifier->_semaphores[EVENT_IN].notify();
      }
      else if (events[i].events & (EPOLLOUT | EPOLLERR | EPOLLHUP))
      {
        event_notifier->_semaphores[EVENT_OUT].notify();
      }
    }
  }
}

auto EpollSelector::registerObject(Descriptor descriptor)
  -> ::std::tuple<EventNotifier *, int>
{
  EventNotifier * event_notifier = _allocateNotifier();
  event_notifier->_descriptor = descriptor;

  epoll_event event;
  event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET;
  event.data.ptr = event_notifier;

  errno = 0;
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, descriptor, &event) != 0)
  {
    _freeNotifier(event_notifier);
    return { nullptr, errno };
  }
  return { event_notifier, 0 };
}

void EpollSelector::deregisterObject(EventNotifier * event_notifier) noexcept
{
  if (event_notifier == nullptr)
  {
    return;
  }

  ::epoll_event event;
  ::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_notifier->_descriptor, &event);
  event_notifier->_descriptor = INVALID_FD;
  _freeNotifier(event_notifier);
  event_notifier = nullptr;
}

auto EpollSelector::_initEpollFd()
  -> int
{
  errno = 0;
  if ((_epoll_fd = ::epoll_create1(0)) == INVALID_FD)
  {
    return errno;
  }
  return 0;
}

void EpollSelector::_destoryEpollFd() noexcept
{
  if (_epoll_fd != INVALID_FD)
  {
    ::close(_epoll_fd);
  }
}

auto EpollSelector::_initSignalFd(::std::uint32_t signal)
  -> int
{
  ::sigset_t mask;
  ::sigemptyset(&mask);
  ::sigaddset(&mask, signal);
  if ((_signal_fd = ::signalfd(-1, &mask, SFD_NONBLOCK)) == INVALID_FD)
  {
    return errno;
  }

  epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = _signal_fd;

  errno = 0;
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _signal_fd, &event) != 0)
  {
    ::close(_signal_fd);
    return errno;
  }
  _signal = signal;
  return 0;
}

void EpollSelector::_destroySignalFd() noexcept
{
  if (_signal_fd != INVALID_FD)
  {
    epoll_event event;
    ::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _signal_fd, &event);
    ::close(_signal_fd);
  }
}

auto EpollSelector::_readSignal()
  -> ::std::uint32_t
{
  ::signalfd_siginfo signal_info;
  ::ssize_t size = ::read(_signal_fd, &signal_info, sizeof(::signalfd_siginfo));
  if (size != sizeof(::signalfd_siginfo))
  {
    return 0;
  }
  return signal_info.ssi_signo;
}

auto EpollSelector::_allocateNotifier()
  -> EventNotifier *
{
  return _notifier_pool.allocate();
}

void EpollSelector::_freeNotifier(EventNotifier * event_notifier)
{
  if (event_notifier == nullptr)
  {
    return;
  }

  for (auto & semaphore : event_notifier->_semaphores)
  {
    semaphore.reset();
  }

  _notifier_pool.free(event_notifier);
}

}
}
}
