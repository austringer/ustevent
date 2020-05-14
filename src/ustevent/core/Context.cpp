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
  : Context(thread_num, DEBUG_OFF)
{}

Context::Context(::std::size_t thread_num, DebugFlag debug_flag)
  : _barrier(thread_num + 1)
  , _strategies(thread_num, nullptr)
  , _debug_flag(debug_flag)
{}

Context::~Context()
{
  terminate();
}

void Context::setStackSize(::std::size_t stack_size)
{
  _stack_size_in_context = stack_size;
}

void Context::start()
{
  _barrier.wait();
}

void Context::terminate()
{
  {
    ::std::unique_lock<thread::Mutex> lock(_done_mutex);
    _done = true;
  }
  _done_cv.notify_all();
}

Context::TaskParameters::TaskParameters() = default;

Context::TaskParameters::TaskParameters(::std::size_t stack_size)
  : _stack_size(stack_size)
{}

Context::TaskParameters::TaskParameters(::std::string description)
  : _description(::std::move(description))
{}

Context::TaskParameters::TaskParameters(::std::size_t stack_size, ::std::string description)
  : _stack_size(stack_size)
  , _description(::std::move(description))
{}

void Context::_notify(::std::size_t index)
{
  if (index < _strategies.size())
  {
    _strategies[index]->notify();
  }
}

void Context::_swapOutPostedFiberTask(::std::list<ContextTask> * list)
{
  assert(list != nullptr);

  list->clear();
  ::std::scoped_lock<thread::Mutex> lock(_fiber_task_list_mutex);
  list->swap(_fiber_task_list);
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
