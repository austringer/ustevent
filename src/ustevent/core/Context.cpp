#include "ustevent/core/Context.h"

#include <cassert>
#include "ustevent/core/thread/detail/CallStack.h"

namespace ustevent
{

Context::Context() = default;

Context::~Context() noexcept = default;

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

}
