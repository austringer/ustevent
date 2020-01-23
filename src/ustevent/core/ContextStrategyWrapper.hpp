#ifndef USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_HPP_
#define USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_HPP_

#include "ustevent/core/ContextStrategyWrapper.h"

namespace ustevent
{

template <typename Strategy>
template <typename ... Args>
ContextStrategyWrapper<Strategy>::ContextStrategyWrapper(Context & context, Args ... args)
  : Strategy(::std::forward<Args>(args)...)
  , _context(context)
{
  _context._strategies.push(this);
}

template <typename Strategy>
auto ContextStrategyWrapper<Strategy>::_launchPostedFiber()
  -> ::std::size_t
{
  ::std::list<Context::ContextTask> posted;
  _context._swapOutPostedFiberTask(&posted);

  ::std::size_t launch_count = posted.size();

  while (!posted.empty())
  {
    auto & top = posted.front();
    if (top._task_operation)
    {
      Fiber f;
      f.setStackSize(top._task_params.stack_size);
      f.setDescription(::std::move(top._task_params.description));
      f.start([operation=::std::move(top._task_operation)](){ operation->perform(); });
      f.detach();
    }
    posted.pop_front();
  }

  return launch_count;
}

template <typename Strategy>
void ContextStrategyWrapper<Strategy>::suspend_until(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  if (_launchPostedFiber() > 0)
  {
    return;
  }

  Strategy::suspend_until(time_point);
}

}

#endif // USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_HPP_
