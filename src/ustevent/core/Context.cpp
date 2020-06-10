#include "ustevent/core/Context.h"

#include <cassert>
#include <boost/fiber/algo/round_robin.hpp>
#include "ustevent/core/thread/detail/CallStack.h"
#include "ustevent/core/ContextStrategy.h"

namespace ustevent
{

Context::Context()
  : Context(1)
{}

Context::Context(::std::size_t thread_num)
  : Context(thread_num, DEBUG_OFF, DEFAULT_BUFFER_LENGTH)
{}

Context::Context(::std::size_t thread_num, DebugFlag debug_flag, ::std::size_t buffer_length)
  : _barrier(thread_num)
  , _strategies(thread_num, nullptr)
  , _debug_flag(debug_flag)
  , _context_task_queue(buffer_length != 0 ? buffer_length : DEFAULT_BUFFER_LENGTH)
{}

Context::~Context()
{
  terminate();
}

void Context::setStackSize(::std::size_t stack_size)
{
  _stack_size_in_context = stack_size;
}

void Context::terminate()
{
  {
    ::std::unique_lock<thread::Mutex> lock(_done_mutex);
    _done = true;
  }
  _done_cv.notify_all();
}

void Context::_notify(::std::size_t index)
{
  if (index < _strategies.size() && _strategies[index] != nullptr)
  {
    _strategies[index]->notify();
  }
}

auto Context::_scheduleOutRemoteTask(detail::Operation * * operation, ::std::size_t * stack_size, ::std::string_view * description)
  -> bool
{
  assert(operation != nullptr);
  assert(stack_size != nullptr);
  assert(description != nullptr);

  return _context_task_queue.pop(operation, stack_size, description);
}

auto Context::_isRunningInThis() const
  -> bool
{
  return thread::CallStack<Context>::contain(static_cast<Context const*>(this));
}

void installMainContext()
{
  boost::fibers::use_scheduling_algorithm<boost::fibers::algo::round_robin>();
}

}
