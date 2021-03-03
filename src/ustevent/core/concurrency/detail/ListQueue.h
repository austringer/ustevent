#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_H_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_H_

#include "ustevent/core/concurrency/detail/ListNodeAllocator.h"

namespace ustevent
{
namespace detail
{

template <typename T>
class ListQueue
{
public:
  ListQueue();

  ~ListQueue() noexcept;

  template <typename R>
  void enqueueSp(R && input);

  template <typename R>
  void enqueueMp(R && input);

  auto dequeueSc(T & output)
    -> bool;

  auto dequeueMc(T & output)
    -> bool;

  auto dequeueMcWeak(T & output)
    -> bool;

  ListQueue(ListQueue const&) = delete;
  auto operator=(ListQueue const&)
    -> ListQueue & = delete;

private:
  using ListNode = typename ListNodeAllocator<T>::ListNode;
  using StorageNode = typename ListNodeAllocator<T>::StorageNode;

  ListNodeAllocator<T>  _allocator;
};

}
}

#include "ustevent/core/concurrency/detail/ListQueue.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_H_
