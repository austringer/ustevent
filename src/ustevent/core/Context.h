#ifndef USTEVENT_CORE_DETAIL_THEATERIMPL_H_
#define USTEVENT_CORE_DETAIL_THEATERIMPL_H_

#include <functional>
#include <list>
#include <type_traits>
#include "ustevent/core/detail/Operation.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/fiber/ConditionVariable.h"
#include "ustevent/core/detail/AppendOnlyArray.h"

namespace ustevent
{

class Context
{
public:
  Context();

  ~Context() noexcept;

  void setStackSize(::std::size_t stack_size);

  template <typename Strategy, typename ... Args>
  void run(Args && ... args);

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
  void _swapOutPostedFiberTask(::std::list<ContextTask> * list);

private:

  auto _isRunningInThis() const
    -> bool;

  template <typename Callable>
  void _postInRemote(Callable && fiber_task, TaskParameters && params);

  mutable thread::Mutex                     _done_mutex;
  mutable fiber::ConditionVariableAny       _done_cv;
  bool                                      _done = false;

  ::std::size_t                             _stack_size_in_context = 1024 * 1024;

  // TODO MPSC
  mutable thread::Mutex                     _fiber_task_list_mutex;
  ::std::list<ContextTask>                  _fiber_task_list;

  detail::AppendOnlyArray<::boost::fibers::algo::algorithm::ptr_t>  _strategies;

  friend class ContextStrategy;

  template <typename Strategy>
  friend class ContextStrategyWrapper;
};

}

#include "ustevent/core/Context.hpp"

#endif // USTEVENT_CORE_DETAIL_THEATERIMPL_H_
