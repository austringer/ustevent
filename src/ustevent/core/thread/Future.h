#ifndef USTEVENT_CORE_THREAD_FUTURE_H_
#define USTEVENT_CORE_THREAD_FUTURE_H_

#include <future>

namespace ustevent
{
namespace thread
{

template <typename R>
using Future        = ::std::future<R>;

template <typename R>
using SharedFuture  = ::std::shared_future<R>;

template <typename R>
using Promise       = ::std::promise<R>;

template <typename Callable>
using PackagedTask  = ::std::packaged_task<Callable>;

using FutureStatus  = ::std::future_status;

template <typename Signature>
void swap(PackagedTask<Signature> & l, PackagedTask<Signature> & r) noexcept
{
  l.swap(r);
}

}
}

#endif // USTEVENT_CORE_THREAD_FUTURE_H_
