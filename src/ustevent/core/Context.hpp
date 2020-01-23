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

  ::std::unique_lock<thread::Mutex> lock(_done_mutex);
  _done_cv.wait(lock, [this](){ return _done; });
}

template <typename Callable>
void Context::post(Callable fiber_task)
{
  post(::std::move(fiber_task), TaskParameters{});
}

template <typename Callable>
void Context::post(Callable fiber_task, TaskParameters params)
{
  if (_isRunningInThis())
  {
    Fiber f;
    f.setPolicy(::boost::fibers::launch::post);
    f.setStackSize(params._stack_size != 0 ? params._stack_size : _stack_size_in_context);
    f.setDescription(::std::move(params._description));
    f.start(::std::move(fiber_task));
    f.detach();
  }
  else
  {
    _postInRemote(::std::move(fiber_task), ::std::move(params));
  }
}

template <typename Callable>
void Context::dispatch(Callable fiber_task)
{
  dispatch(::std::move(fiber_task), TaskParameters());
}

template <typename Callable>
void Context::dispatch(Callable fiber_task, TaskParameters params)
{
  if (_isRunningInThis())
  {
    Fiber f;
    f.setPolicy(::boost::fibers::launch::dispatch);
    f.setStackSize(params._stack_size != 0 ? params._stack_size : _stack_size_in_context);
    f.setDescription(::std::move(params._description));
    f.start(::std::move(fiber_task));
    f.detach();
  }
  else
  {
    _postInRemote(::std::move(fiber_task), ::std::move(params));
  }
}

template <typename Callable>
auto Context::call(Callable fiber_task)
  -> ::std::invoke_result_t<Callable>
{
  return call(::std::move(fiber_task), TaskParameters());
}

template <typename Callable>
auto Context::call(Callable fiber_task, TaskParameters params)
  -> ::std::invoke_result_t<Callable>
{
  if (_isRunningInThis())
  {
    return fiber_task();
  }
  else
  {
    fiber::PackagedTask<::std::invoke_result_t<Callable>()> packaged_task(::std::move(fiber_task));
    auto future_result = packaged_task.get_future();

    _postInRemote(::std::move(packaged_task), ::std::move(params));

    return future_result.get();
  }
}

template <typename Callable, typename Parameters>
Context::ContextTask::ContextTask(Callable && fiber_task, Parameters && params)
  : _task_operation(
      std::make_unique<detail::ContextOperation<::std::decay_t<Callable>>>(
        std::forward<Callable>(fiber_task)
      )
    )
  , _task_params(::std::forward<Parameters>(params))
{}

template <typename Callable>
void Context::_postInRemote(Callable && fiber_task, TaskParameters && params)
{
  {
    ::std::scoped_lock<thread::Mutex> lock(_fiber_task_list_mutex);
    _fiber_task_list.emplace_back(::std::move(fiber_task), ::std::move(params));
  }
  // TODO find idle thread
  if (_strategies.size() > 0)
  {
    _strategies.at(0)->notify();
  }
}

}

#endif // USTEVENT_CORE_DETAIL_THEATERIMPL_HPP_
