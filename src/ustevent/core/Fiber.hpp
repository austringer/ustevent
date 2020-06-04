#ifndef USTEVENT_CORE_FIBER_HPP_
#define USTEVENT_CORE_FIBER_HPP_

#include "ustevent/core/Fiber.h"
#include "boost/fiber/operations.hpp"
#include "boost/fiber/protected_fixedsize_stack.hpp"
#include "ustevent/core/FiberDebugInfo.h"

namespace ustevent
{

// a hack version of boost::fibers::fiber constructor, see fiber/src/fiber.cpp
template <typename Fn, typename ... Args>
void Fiber::start(Fn && function, Args ... args)
{
  auto f = [function=::std::forward<Fn>(function), args=::std::make_tuple(::std::forward<Args>(args)...)]() mutable
  {
    ::std::apply(::std::move(function), ::std::move(args));

    // The fiber is going to finish. ContextStrategy will omit the fibers which the _terminated is true
    FiberDebugInfo & debug_info = ::boost::this_fiber::properties<FiberDebugInfo>();
    debug_info.setTerminated(true);
  };

  if (_stack_size > 0)
  {
    _context = ::boost::fibers::make_worker_context(
      _policy,
      ::boost::fibers::protected_fixedsize_stack(_stack_size),
      ::std::move(f));
  }
  else
  {
    _context = ::boost::fibers::make_worker_context(
      _policy,
      ::boost::fibers::default_stack(),
      ::std::move(f));
  }

  // alloc fiber properties before fiber stareted
  auto debug_info_ptr = _allocFiberDebugInfo(_context.get());
  debug_info_ptr->setDescription(::std::move(_description));
  _context->set_properties(debug_info_ptr);

  auto active_fcontext = ::boost::fibers::context::active();
  active_fcontext->attach(_context.get());
  switch (_policy)
  {
  case ::boost::fibers::launch::post:
    active_fcontext->get_scheduler()->schedule(_context.get());
    break;
  case ::boost::fibers::launch::dispatch:
    _context->resume(active_fcontext);
    break;
  default:
    assert(false);
  }
}

}

#endif // USTEVENT_CORE_FIBER_HPP_
