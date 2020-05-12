#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_HPP_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_HPP_

#include "ustevent/core/concurrency/detail/ListQueue.h"

namespace ustevent
{
namespace detail
{

template <typename T>
ListQueue<T>::ListQueue() = default;

template <typename T>
ListQueue<T>::~ListQueue() noexcept = default;

template <typename T>
template <typename R>
void ListQueue<T>::enqueueSp(R && input)
{
  ListNode * node = _allocator.alloc();
  node->_data = ::std::forward<R>(input);
  _allocator.head().load(::std::memory_order_relaxed)->_next.store(node, ::std::memory_order_release);
  _allocator.head().store(node, ::std::memory_order_relaxed);
}

template <typename T>
template <typename R>
void ListQueue<T>::enqueueMp(R && input)
{
  ListNode * node = _allocator.alloc()();
  node->_data = ::std::forward<R>(input);
  ListNode * prev_head = _allocator.head().exchange(node, ::std::memory_order_acq_rel);
  prev_head->_next.store(node, ::std::memory_order_release);
}

template <typename T>
auto ListQueue<T>::dequeueSc(T & output)
  -> bool
{
  ListNode * tail = _allocator.tail().load(::std::memory_order_relaxed);
  ListNode * next = tail->_next.load(::std::memory_order_acquire);
  if (next == nullptr)
  {
    return false;
  }
  output = ::std::move(next->_data);
  _allocator.tail().store(next, ::std::memory_order_release);
  _allocator.free(tail);
  return true;
}

template <typename T>
auto ListQueue<T>::dequeueMc(T & output)
  -> bool
{
  ListNode * tail = _allocator.tail().exchange(nullptr, ::std::memory_order_acq_rel);
  while (tail == nullptr)
  {
    ::std::this_thread::yield();
    tail = _allocator.tail().exchange(nullptr, ::std::memory_order_acq_rel);
  }
  ListNode * next = tail->_next.load(::std::memory_order_acquire);
  if (next == nullptr)
  {
    _allocator.tail().exchange(tail, ::std::memory_order_acq_rel);
    return false;
  }
  output = ::std::move(next->_data);
  _allocator.tail().store(next, ::std::memory_order_release);
  _allocator.free(tail);
  return true;
}

template <typename T>
auto ListQueue<T>::dequeueMcWeak(T & output)
  -> bool
{
  ListNode * tail = _allocator.tail().exchange(nullptr, ::std::memory_order_acq_rel);
  if (tail == nullptr)
  {
    return false;
  }
  ListNode * next = tail->_next.load(::std::memory_order_acquire);
  if (next == nullptr)
  {
    _allocator.tail().exchange(tail, ::std::memory_order_acq_rel);
    return false;
  }
  output = ::std::move(next->_data);
  _allocator.tail().store(next, ::std::memory_order_release);
  _allocator.free(tail);
  return true;
}

}
}

#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_LISTQUEUE_HPP_
