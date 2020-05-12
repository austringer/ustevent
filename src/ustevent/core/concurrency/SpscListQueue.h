#ifndef USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_H_
#define USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_H_

#include "ustevent/core/concurrency/detail/ListQueue.h"

namespace ustevent
{

template <typename T>
class SpscListQueue
{
public:
  SpscListQueue();

  ~SpscListQueue() noexcept;

  SpscListQueue(SpscListQueue const&) = delete;

  SpscListQueue<T> & operator=(SpscListQueue const&) = delete;

  void enqueue(T const& input);

  void enqueue(T && input);

  auto dequeue(T & output)
    -> bool;

private:
  detail::ListQueue<T> _list_queue;
};

}

#include "ustevent/core/concurrency/SpscListQueue.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_H_
