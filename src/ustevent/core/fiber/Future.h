#ifndef USTEVENT_CORE_FIBERP_FUTURE_H_
#define USTEVENT_CORE_FIBERP_FUTURE_H_

#include "boost/fiber/future.hpp"

namespace ustevent
{
namespace fiber
{

template <typename R>
using Future        = ::boost::fibers::future<R>;

template <typename R>
using SharedFuture  = ::boost::fibers::shared_future<R>;

template <typename R>
using Promise       = ::boost::fibers::promise<R>;

template <typename Callable>
using PackagedTask  = ::boost::fibers::packaged_task<Callable>;

using FutureStatus  = ::boost::fibers::future_status;

template <typename Signature>
void swap(PackagedTask<Signature> & l, PackagedTask<Signature> & r) noexcept
{
  l.swap(r);
}

}
}

#endif // USTEVENT_CORE_FIBER_FUTURE_H_
