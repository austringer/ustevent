#ifndef USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_
#define USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_

#include <cstddef>
#include <thread>

namespace ustevent
{
namespace detail
{

template <typename T, ::std::size_t Alloc>
ObjectPool<T, Alloc>::ObjectPool()
  : _storage_head(new StorageNode)
  , _storage_tail(_storage_head.load(::std::memory_order_relaxed))
{}

template <typename T, ::std::size_t Alloc>
ObjectPool<T, Alloc>::~ObjectPool() noexcept
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

template <typename T, ::std::size_t Alloc>
auto ObjectPool<T, Alloc>::alloc()
  -> T *
{
  ListNode * node = _tryDequeueFreeList();
  if (node == nullptr)
  {
    //pre-allocate
    ::std::vector<ListNode> new_nodes(Alloc);

    //store the sequencing chain here before it goes on the freelist
    for (::std::size_t i = 2; i < Alloc; ++i)
    {
      new_nodes[i]._next.store(&new_nodes[i - 1], ::std::memory_order_relaxed);
    }

    //connect the chains
    ListNode * free_list_prev_head = _free_list_head.exchange(&new_nodes[1], ::std::memory_order_acq_rel);
    free_list_prev_head->next.store(&new_nodes[Alloc - 1], ::std::memory_order_release);

    //the first one is reserved for this alloc call
    node = &new_nodes[0];

    //store the node on the storage queue for retrieval later
    StorageNode * new_storage = new StorageNode(::std::move(new_nodes));
    StorageNode * prev_head = _storage_head.exchange(new_storage, std::memory_order_acq_rel);
    prev_head->_next.store(new_storage, ::std::memory_order_release);
  }
  return &node->_data;
}

template <typename T, ::std::size_t Alloc>
void ObjectPool<T, Alloc>::free(T * object)
{
  constexpr auto offset = offsetof(ListNode, _data);
  ListNode * node = reinterpret_cast<ListNode *>(reinterpret_cast<char *>(object) - offset);
  node->_next.store(nullptr, ::std::memory_order_relaxed);
  ListNode * free_list_prev_head = _free_list_head.exchange(node, ::std::memory_order_acq_rel);
  free_list_prev_head->_next.store(node, ::std::memory_order_release);
}

template <typename T, ::std::size_t Alloc>
auto ObjectPool<T, Alloc>::_tryDequeueFreeList()
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
    node->next.store(nullptr, std::memory_order_relaxed);
    return node;
  }
}

template <typename T, ::std::size_t Alloc>
template <typename R>
ObjectPool<T, Alloc>::ListNode::ListNode(R && input)
  : _data(std::forward<R>(input))
{}

template <typename T, ::std::size_t Alloc>
template <typename R>
ObjectPool<T, Alloc>::StorageNode::StorageNode(R && input)
  : _nodes(std::forward<R>(input))
{}

}
}


#endif // USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_
