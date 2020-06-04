#include "ustevent/core/ContextStrategy.h"

#include <random>

#include "boost/fiber/context.hpp"
#include "boost/fiber/type.hpp"
#include "ustevent/core/detail/Cache.h"
#include "ustevent/core/Context.h"

namespace ustevent
{

ContextStrategy::ContextStrategy(Context & context)
  : _context(context)
  , _id(_context._next_strategy_index++)
  , _waiting_fibers_head(nullptr)
{
  _context._strategies[_id] = this;
}

ContextStrategy::~ContextStrategy() noexcept = default;

void ContextStrategy::awakened(::boost::fibers::context * context, FiberDebugInfo & debug_info) noexcept
{
  assert(context);
  if (!context->is_context(::boost::fibers::type::pinned_context))
  {
    context->detach();

    if (_context._debug_flag == Context::DEBUG_ON)
    {
      _eraseBacktrace(debug_info);
    }
  }
  _ready_queue.push(context);
}

auto ContextStrategy::pick_next() noexcept
  -> ::boost::fibers::context *
{
  ::boost::fibers::context * victim = _ready_queue.pop();
  if (victim)
  {
    Cache::prefetchForWriting(victim, sizeof(::boost::fibers::context));
    if (!victim->is_context(::boost::fibers::type::pinned_context))
    {
      ::boost::fibers::context::active()->attach(victim);
    }
  }
  else
  {
    ::std::size_t strategy_count = _context._strategies.size();
    if (strategy_count > 1)
    {
      // It's meaningless and bugged to steal do stealing
      // in the context with only one strategy.
      ::std::size_t next_scheduler_id = 0;
      ::std::size_t i = 0;
      static thread_local ::std::minstd_rand generator{ std::random_device{}() };
      ::std::uniform_int_distribution<::std::size_t> distribution{ 0, strategy_count - 1 };
      do
      {
        do
        {
          ++i;
          next_scheduler_id = distribution(generator);
        } while (next_scheduler_id == _id);
        // steal ready fiber from other strategy
        victim = _context._strategies[next_scheduler_id]->steal();
      } while (victim == nullptr && i < strategy_count);
      if (victim)
      {
        Cache::prefetchForWriting(victim, sizeof(::boost::fibers::context));
        assert(!victim->is_context(::boost::fibers::type::pinned_context));
        ::boost::fibers::context::active()->attach(victim);
      }
    }
  }
  if (_context._debug_flag == Context::DEBUG_ON &&
      victim &&
      ::boost::fibers::context::active()->is_context(::boost::fibers::type::worker_context))
  {
    _recordBacktrace();
  }
  return victim;
}

auto ContextStrategy::has_ready_fibers() const noexcept
  -> bool
{
  return !_ready_queue.empty();
}

void ContextStrategy::suspend_until(::std::chrono::steady_clock::time_point const& time_point) noexcept
{
  if (_launchPostedTask() > 0)
  {
    return;
  }

  onSuspended(time_point);
}

void ContextStrategy::notify() noexcept
{
  onNotified();
}

auto ContextStrategy::steal() noexcept
  -> ::boost::fibers::context *
{
  return _ready_queue.steal();
}

auto ContextStrategy::_launchPostedTask()
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

void ContextStrategy::_recordBacktrace()
{
  ::boost::fibers::context * active_fcontext = ::boost::fibers::context::active();

  // Directly alloc properties instead of calling this_fiber::properties() here if the fiber is dispatched.
  // In this_fiber::properties(), a yield() will be called if the properties is unset.
  // Then the scheduler then try to pick next ready context.
  // That means in some case the current active context, of cause is ready,
  // will be picked and try to dereference from a nullptr.
  ::boost::fibers::fiber_properties * properties = active_fcontext->get_properties();
  if (properties == nullptr)
  {
    properties = new_properties(active_fcontext);
    super::set_properties(active_fcontext, properties);
  }
  FiberDebugInfo * debug_info = static_cast<FiberDebugInfo *>(properties);
  assert(debug_info);
  if (!debug_info->terminated())
  {
    debug_info->updateFrame();
    // append to the waiting fibers list
    _waiting_fibers_tail->_next = debug_info;
    debug_info->_prev = _waiting_fibers_tail;
    _waiting_fibers_tail = debug_info;
  }
}

void ContextStrategy::_eraseBacktrace(FiberDebugInfo & debug_info)
{
  // remove the context debug info from waiting list
  if (debug_info._prev || debug_info._next)
  {
    if (debug_info._next == nullptr)
    {
      _waiting_fibers_tail = debug_info._prev;
      _waiting_fibers_tail->_next = nullptr;
    }
    else
    {
      debug_info._prev->_next = debug_info._next;
      debug_info._next->_prev = debug_info._prev;
    }
    debug_info._prev = nullptr;
    debug_info._next = nullptr;
  }
}

}
