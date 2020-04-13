#ifndef USTEVENT_CORE_CONTEXTSTRATEGY_H_
#define USTEVENT_CORE_CONTEXTSTRATEGY_H_

#include "boost/fiber/algo/algorithm.hpp"
#include "boost/fiber/detail/context_spmc_queue.hpp"
#include "ustevent/core/detail/Macros.h"
#include "ustevent/core/FiberDebugInfo.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/thread/ConditionVariable.h"

namespace ustevent
{

class Context;

class ContextStrategy : public ::boost::fibers::algo::algorithm_with_properties<FiberDebugInfo>
{
public:
  explicit
  ContextStrategy(Context & context);

  ~ContextStrategy() noexcept override;

  void awakened(::boost::fibers::context * context, FiberDebugInfo & debug_info) noexcept override;

  auto pick_next() noexcept
    -> ::boost::fibers::context * override;

  auto has_ready_fibers() const noexcept
    -> bool override;

  void suspend_until(::std::chrono::steady_clock::time_point const& time_point) noexcept final;

  void notify() noexcept override;

  auto steal() noexcept
    -> ::boost::fibers::context *;

  ContextStrategy(ContextStrategy const&) = delete;
  auto operator=(ContextStrategy const&)
    -> ContextStrategy & = delete;

protected:

  virtual void onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept = 0;

  virtual void onNotified() noexcept = 0;

private:
  const ::std::uint64_t         _magic = 0x544E455645545355;  // "USTEVENT" in ascii

  Context &                     _context;

  ::std::size_t                 _id;

  FiberDebugInfo                _waiting_fibers_head;
  FiberDebugInfo *              _waiting_fibers_tail = &_waiting_fibers_head;

  using ContextSpmcQueue =      ::boost::fibers::detail::context_spmc_queue;
  ContextSpmcQueue              _ready_queue;

  auto _launchPostedFiber()
    -> ::std::size_t;

  void _recordBacktrace();

  void _eraseBacktrace(FiberDebugInfo & debug_info);
};

}

#endif // USTEVENT_CORE_CONTEXTSTRATEGY_H_
