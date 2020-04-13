#ifndef USTEVENT_CORE_BLOCKINGCONTEXTSTRATEGY_H_
#define USTEVENT_CORE_BLOCKINGCONTEXTSTRATEGY_H_

#include "ustevent/core/ContextStrategy.h"

namespace ustevent
{

class BlockingContextStrategy : public ContextStrategy
{
public:
  explicit
  BlockingContextStrategy(Context & context);

  enum BlockingFlag
  {
    BLOCKING,
    POLLING,
  };

  BlockingContextStrategy(Context & context, BlockingFlag blocking_flag);

  ~BlockingContextStrategy() noexcept override;

protected:
  void onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept override;

  void onNotified() noexcept override;

private:
  BlockingFlag                          _blocking_flag;
  bool                                  _notified = false;

  mutable thread::Mutex                 _suspend_mutex;
  mutable thread::ConditionVariable     _suspend_cv;
};

}

#endif // USTEVENT_CORE_BLOCKINGCONTEXTSTRATEGY_H_
