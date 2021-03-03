#ifndef USTEVENT_CORE_DETAIL_THEATERIMPL_HPP_
#define USTEVENT_CORE_DETAIL_THEATERIMPL_HPP_

#include "ustevent/core/Context.h"

#include <boost/fiber/operations.hpp>
#include "ustevent/core/detail/ContextOperation.h"
#include "ustevent/core/Fiber.h"
#include "ustevent/core/thread/detail/CallStack.h"
#include "ustevent/core/fiber/Future.h"

namespace ustevent
{

template <typename Strategy, typename ... Args>
void Context::run(Args && ... args)
{
  typename thread::CallStack<Context>::Context context(this);

  ::boost::fibers::use_scheduling_algorithm<Strategy>(::std::forward<Args>(args)...);

  _barrier.wait();

  ::std::unique_lock<thread::Mutex> lock(_done_mutex);
  _done_cv.wait(lock, [this](){ return _done; });
}

template <typename Callable>
void Context::post(Callable fiber_task)
{
  post(::std::move(fiber_task), 0, "");
}

template <typename Callable>
void Context::post(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description)
{
  if (_isRunningInThis())
  if (isRunningInThis())
  {
    Fiber f;
    f.setPolicy(::boost::fibers::launch::post);
    f.setStackSize(stack_size != 0 ? stack_size : _stack_size_in_context);
    if (_debug_flag == DEBUG_ON)
    {
      f.setDescription(description);
    }
    f.start(::std::move(fiber_task));
    f.detach();
  }
  else
  {
    _postTaskFromRemote(::std::move(fiber_task), stack_size, description);
  }
}

template <typename Callable>
void Context::dispatch(Callable fiber_task)
{
  dispatch(::std::move(fiber_task), 0, "");
}

template <typename Callable>
void Context::dispatch(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description)
{
  if (isRunningInThis())
  {
    Fiber f;
    f.setPolicy(::boost::fibers::launch::dispatch);
    f.setStackSize(stack_size != 0 ? stack_size : _stack_size_in_context);
    if (_debug_flag == DEBUG_ON)
    {
      f.setDescription(description);
    }
    f.start(::std::move(fiber_task));
    f.detach();
  }
  else
  {
    _postTaskFromRemote(::std::move(fiber_task), stack_size, description);
  }
}

template <typename Callable>
auto Context::call(Callable fiber_task)
  -> ::std::invoke_result_t<Callable>
{
  return call(::std::move(fiber_task), 0, "");
}

template <typename Callable>
auto Context::call(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description)
  -> ::std::invoke_result_t<Callable>
{
  if (isRunningInThis())
  {
    return fiber_task();
  }
  else
  {
    fiber::PackagedTask<::std::invoke_result_t<Callable>()> packaged_task(::std::move(fiber_task));
    auto future_result = packaged_task.get_future();

    _postTaskFromRemote(::std::move(packaged_task), stack_size, description);

    return future_result.get();
  }
}

template <typename Callable>
void Context::_postTaskFromRemote(Callable && fiber_task, ::std::size_t stack_size, ::std::string_view description)
{
  using H = ::std::decay_t<Callable>;
  using D = detail::ContextTaskQueue::TaskDeallocator;
  if (_debug_flag == DEBUG_OFF)
  {
    _context_task_queue.push(detail::ContextOperation<H, D>(::std::forward<Callable>(fiber_task), D(_context_task_queue)), stack_size, "");
  }
  else
  {
    _context_task_queue.push(detail::ContextOperation<H, D>(::std::forward<Callable>(fiber_task), D(_context_task_queue)), stack_size, description);
  }
  // TODO find idle thread
  _notify(0);
}

}

#endif // USTEVENT_CORE_DETAIL_THEATERIMPL_HPP_
