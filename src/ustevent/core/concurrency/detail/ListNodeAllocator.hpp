#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_HPP_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_HPP_

#include <cstddef>
#include <thread>

#include "ustevent/core/concurrency/detail/ListNodeAllocator.h"

namespace ustevent
{
namespace detail
{


template <typename T>
ListNodeAllocator<T>::ListNodeAllocator(::std::size_t alloc_size)
  : _alloc_size(alloc_size)
{
  ::std::vector<ListNode> vec(2);
  _head.store(&vec[0]);
  _tail.store(_head.load(::std::memory_order_relaxed), ::std::memory_order_relaxed);

  _free_list_head.store(&vec[1]);
  _free_list_tail.store(_free_list_head.load(::std::memory_order_relaxed), ::std::memory_order_relaxed);

  StorageNode * store = new StorageNode(::std::move(vec));
  StorageNode * prev_head = _storage_head.exchange(store, ::std::memory_order_acq_rel);
  prev_head->_next.store(store, ::std::memory_order_release);
}

template <typename T>
ListNodeAllocator<T>::ListNodeAllocator(NoHeadTail /*tag*/, ::std::size_t alloc_size)
  : _alloc_size(alloc_size)
  , _storage_head(new StorageNode)
  , _storage_tail(_storage_head.load(::std::memory_order_relaxed))
{}

template <typename T>
ListNodeAllocator<T>::~ListNodeAllocator() noexcept
{
  StorageNode * tail = _storage_tail.load(::std::memory_order_relaxed);
  StorageNode * next = tail->_next.load(::std::memory_order_relaxed);
  while (next != nullptr)
  {
    tail = next;
    next = next->_next.load(::std::memory_order_relaxed);
    delete tail;
  }
}

template <typename T>
auto ListNodeAllocator<T>::head()
  -> ::std::atomic<ListNode *> &
{
  return _head;
}

template <typename T>
auto ListNodeAllocator<T>::tail()
  -> ::std::atomic<ListNode *> &
{
  return _tail;
}

template <typename T>
auto ListNodeAllocator<T>::alloc()
  -> ListNode *
{
  ListNode * node = _tryDequeueFreeList();
  if (node == nullptr)
  {
    //pre-allocate
    ::std::vector<ListNode> new_nodes(_alloc_size);

    //store the sequencing chain here before it goes on the freelist
    for (::std::size_t i = 2; i < _alloc_size; ++i)
    {
      new_nodes[i]._next.store(&new_nodes[i - 1], ::std::memory_order_relaxed);
    }

    //connect the chains
    ListNode * free_list_prev_head = _free_list_head.exchange(&new_nodes[1], ::std::memory_order_acq_rel);
    free_list_prev_head->_next.store(&new_nodes[_alloc_size - 1], ::std::memory_order_release);

    //the first one is reserved for this alloc call
    node = &new_nodes[0];

    //store the node on the storage queue for retrieval later
    StorageNode * new_storage = new StorageNode(::std::move(new_nodes));
    StorageNode * prev_head = _storage_head.exchange(new_storage, std::memory_order_acq_rel);
    prev_head->_next.store(new_storage, ::std::memory_order_release);
  }
  return node;
}

template <typename T>
void ListNodeAllocator<T>::free(ListNode * node)
{
  node->_next.store(nullptr, ::std::memory_order_relaxed);
  ListNode * free_list_prev_head = _free_list_head.exchange(node, ::std::memory_order_acq_rel);
  free_list_prev_head->_next.store(node, ::std::memory_order_release);
}

template <typename T>
auto ListNodeAllocator<T>::_tryDequeueFreeList()
  -> ListNode *
{
  ListNode * node = _free_list_tail.exchange(nullptr, std::memory_order_acq_rel);
  while (node == nullptr)
  {
    ::std::this_thread::yield();
    node = _free_list_tail.exchange(nullptr, std::memory_order_acq_rel);
  }

  ListNode * next = node->_next.load(::std::memory_order_acquire);
  if (next == nullptr)
  {
    _free_list_tail.store(node, ::std::memory_order_release);
    return nullptr;
  }
  else
  {
    _free_list_tail.store(next, ::std::memory_order_release);
    node->_next.store(nullptr, std::memory_order_relaxed);
    return node;
  }
}

template <typename T>
template <typename R>
ListNodeAllocator<T>::ListNode::ListNode(R && input)
  : _data(std::forward<R>(input))
{}

template <typename T>
template <typename R>
ListNodeAllocator<T>::StorageNode::StorageNode(R && input)
  : _nodes(std::forward<R>(input))
{}

}
}


#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_HPP_
