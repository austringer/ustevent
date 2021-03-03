#ifndef USTEVENT_CORE_DETAIL_THEATERIMPL_H_
#define USTEVENT_CORE_DETAIL_THEATERIMPL_H_

#include <atomic>
#include <list>
#include <vector>
#include <type_traits>  // invoke_result
#include <boost/intrusive_ptr.hpp>
#include "ustevent/core/detail/Operation.h"
#include "ustevent/core/detail/SpinMutex.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/thread/Barrier.h"
#include "ustevent/core/fiber/ConditionVariable.h"
#include "ustevent/core/detail/ContextTaskQueue.h"

namespace ustevent
{

class ContextStrategy;

class Context
{
public:
  Context();

  explicit
  Context(::std::size_t thread_num);

  enum DebugFlag
  {
    DEBUG_ON,
    DEBUG_OFF,
  };

  enum : ::std::size_t
  {
    DEFAULT_BUFFER_LENGTH = 16 * 1024
  };

  Context(::std::size_t thread_num, DebugFlag debug_flag, ::std::size_t buffer_length);

  ~Context() noexcept;

  void setStackSize(::std::size_t stack_size);

  template <typename Strategy, typename ... Args>
  void run(Args && ... args);

  auto isRunningInThis() const
    -> bool;

  void terminate();

  template <typename Callable>
  void post(Callable fiber_task);

  template <typename Callable>
  void post(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description);

  template <typename Callable>
  void dispatch(Callable fiber_task);

  template <typename Callable>
  void dispatch(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description);

  template <typename Callable>
  auto call(Callable fiber_task)
    -> ::std::invoke_result_t<Callable>;

  template <typename Callable>
  auto call(Callable fiber_task, ::std::size_t stack_size, ::std::string_view description)
    -> ::std::invoke_result_t<Callable>;

  Context(Context const&) = delete;
  auto operator=(Context const&)
    -> Context & = delete;

private:

  template <typename Callable>

  void _postTaskFromRemote(Callable && fiber_task, ::std::size_t stack_size, ::std::string_view description);

  void _notify(::std::size_t index);

  auto _scheduleOutRemoteTask(detail::Operation * * operation, ::std::size_t * stack_size, ::std::string_view * description)
    -> bool;

  mutable thread::Mutex                     _done_mutex;
  mutable fiber::ConditionVariableAny       _done_cv;
  bool                                      _done = false;

  ::std::size_t                             _stack_size_in_context = 1024 * 1024;

  thread::Barrier                           _barrier;
  ::std::atomic_size_t                      _next_strategy_index = { 0 };
  ::std::vector<::boost::intrusive_ptr<ContextStrategy>>    _strategies;

  DebugFlag                                 _debug_flag;

  detail::ContextTaskQueue                  _context_task_queue;

  friend class ContextStrategy;

  template <typename Strategy>
  friend class ContextStrategyWrapper;
};

void installMainContext();

}

#include "ustevent/core/Context.hpp"

#endif // USTEVENT_CORE_DETAIL_THEATERIMPL_H_
