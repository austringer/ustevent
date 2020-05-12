#ifndef USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_H_
#define USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_H_

#include <atomic>
#include <vector>
#include "ustevent/core/concurrency/detail/ListNodeAllocator.h"

namespace ustevent
{

template <typename T>
class ObjectPool
{
public:
  ObjectPool();

  ~ObjectPool() noexcept;

  ObjectPool(ObjectPool const&) = delete;

  ObjectPool<T> & operator=(ObjectPool const&) = delete;

  auto alloc()
    -> T *;

  void free(T * object);

private:
  using ListNode = typename detail::ListNodeAllocator<T>::ListNode;
  using StorageNode = typename detail::ListNodeAllocator<T>::StorageNode;

  detail::ListNodeAllocator<T> _allocator;
};

}

#include "ustevent/core/concurrency/ObjectPool.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_H_
