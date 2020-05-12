#ifndef USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_H_
#define USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_H_

#include "ustevent/core/concurrency/detail/ListQueue.h"

namespace ustevent
{

template <typename T>
class SpmcListQueue
{
public:
  SpmcListQueue();

  ~SpmcListQueue() noexcept;

  SpmcListQueue(SpmcListQueue const&) = delete;

  SpmcListQueue<T> & operator=(SpmcListQueue const&) = delete;

  void enqueue(T const& input);

  void enqueue(T && input);

  auto dequeue(T & output)
    -> bool;

  auto dequeueWeak(T & output)
    -> bool;

private:
  detail::ListQueue<T> _list_queue;
};

}

#include "ustevent/core/concurrency/SpmcListQueue.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_H_
