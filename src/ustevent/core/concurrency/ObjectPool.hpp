#ifndef USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_HPP_
#define USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_HPP_

#include <cstddef>
#include <thread>

#include "ustevent/core/concurrency/ObjectPool.h"

namespace ustevent
{

template <typename T>
ObjectPool<T>::ObjectPool()
  : detail::ListNodeAllocator(detail::ListNodeAllocator::NoHeadTail{})
{}

template <typename T>
ObjectPool<T>::~ObjectPool() noexcept = default;

template <typename T>
auto ObjectPool<T>::alloc()
  -> T *
{
  ListNode * node = _allocator->alloc();
  return &(node->_data);
}

template <typename T>
void ObjectPool<T>::free(T * object)
{
  constexpr auto offset = offsetof(ListNode, _data);
  ListNode * node = reinterpret_cast<ListNode *>(reinterpret_cast<char *>(object) - offset);
  _allocator->free(node);
}

}


#endif // USTEVENT_CORE_CONCURRENCY_OBJECTPOOL_HPP_
