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
auto ContextStrategyWrapper<Strategy>::_launchPostedTask()
  -> ::std::size_t
{
  detail::Operation * operation;
  ::std::size_t stack_size;
  ::std::string_view description;
  if (_context._scheduleOutRemoteTask(&operation, &stack_size, &description))
  {
    assert(operation != nullptr);
    Fiber f;
    f.setStackSize(stack_size);
    f.setDescription(description);
    f.start([operation](){ operation->perform(); });
    f.detach();
    return 1;
  }
  return 0;
}

template <typename Strategy>
void ContextStrategyWrapper<Strategy>::suspend_until(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  if (_launchPostedTask() > 0)
  {
    return;
  }

  Strategy::suspend_until(time_point);
}

}

#endif // USTEVENT_CORE_CONTEXTSTRATEGYWRAPPER_HPP_
