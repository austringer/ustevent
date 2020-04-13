#ifndef USTEVENT_CORE_DETAIL_THEATERIMPL_H_
#define USTEVENT_CORE_DETAIL_THEATERIMPL_H_

// #include <functional>
#include <atomic>
#include <list>
#include <vector>
#include <type_traits>  // invoke_result
#include <boost/intrusive_ptr.hpp>
#include "ustevent/core/detail/Operation.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/thread/Barrier.h"
#include "ustevent/core/fiber/ConditionVariable.h"
#include "ustevent/core/detail/AppendOnlyArray.h"

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

  Context(::std::size_t thread_num, DebugFlag debug_flag);

  ~Context() noexcept;

  void setStackSize(::std::size_t stack_size);

  template <typename Strategy, typename ... Args>
  void run(Args && ... args);

  void run();

  void terminate();

  struct TaskParameters
  {
    TaskParameters();

    explicit
    TaskParameters(::std::size_t stack_size);

    explicit
    TaskParameters(::std::string description);

    TaskParameters(::std::size_t, ::std::string description);

    ::std::size_t   _stack_size = 0;
    ::std::string   _description;
  };

  template <typename Callable>
  void post(Callable fiber_task);

  template <typename Callable>
  void post(Callable fiber_task, TaskParameters params);

  template <typename Callable>
  void dispatch(Callable fiber_task);

  template <typename Callable>
  void dispatch(Callable fiber_task, TaskParameters params);

  template <typename Callable>
  auto call(Callable fiber_task)
    -> ::std::invoke_result_t<Callable>;

  template <typename Callable>
  auto call(Callable fiber_task, TaskParameters params)
    -> ::std::invoke_result_t<Callable>;

  Context(Context const&) = delete;
  auto operator=(Context const&)
    -> Context & = delete;

protected:

  struct ContextTask
  {
    ::std::unique_ptr<detail::Operation>  _task_operation;
    TaskParameters                        _task_params;

    template <typename Callable, typename Parameters>
    ContextTask(Callable && fiber_task, Parameters && params);
  };

private:

  auto _isRunningInThis() const
    -> bool;

  template <typename Callable>
  void _postInRemote(Callable && fiber_task, TaskParameters && params);

  void _notify(::std::size_t index);

  void _swapOutPostedFiberTask(::std::list<ContextTask> * list);

  mutable thread::Mutex                     _done_mutex;
  mutable fiber::ConditionVariableAny       _done_cv;
  bool                                      _done = false;

  ::std::size_t                             _stack_size_in_context = 1024 * 1024;

  // TODO MPSC
  mutable thread::Mutex                     _fiber_task_list_mutex;
  ::std::list<ContextTask>                  _fiber_task_list;

  // detail::AppendOnlyArray<::boost::fibers::algo::algorithm::ptr_t>  _strategies;
  thread::Barrier                           _barrier;
  ::std::atomic_size_t                      _next_strategy_index = { 0 };
  ::std::vector<::boost::intrusive_ptr<ContextStrategy>>    _strategies;

  DebugFlag                                 _debug_flag;

  friend class ContextStrategy;

  template <typename Strategy>
  friend class ContextStrategyWrapper;
};

void installMainContext();

}

#include "ustevent/core/Context.hpp"

#endif // USTEVENT_CORE_DETAIL_THEATERIMPL_H_
