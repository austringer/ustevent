#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_H_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_H_

#include <atomic>
#include <vector>
#include "ustevent/core/detail/Cache.h"

namespace ustevent
{
namespace detail
{

template <typename T>
class ListNodeAllocator
{
public:
  struct ListNode
  {
    T                             _data;
    ::std::atomic<ListNode *>     _next = { nullptr };

    template <typename R>
    ListNode(R && input);

    ListNode() = default;
  };

  struct StorageNode
  {
    ::std::vector<ListNode>       _nodes;
    ::std::atomic<StorageNode *>  _next = { nullptr };

    template <typename R>
    StorageNode(R && input);

    StorageNode() = default;
  };

  explicit
  ListNodeAllocator(::std::size_t alloc_size = 32);

  struct NoHeadTail {};
  ListNodeAllocator(NoHeadTail tag, ::std::size_t _alloc_size = 32);

  ~ListNodeAllocator() noexcept;

  ListNodeAllocator(ListNodeAllocator const&) = delete;
  auto operator=(ListNodeAllocator const&)
    -> ListNodeAllocator & = delete;

  auto head()
    -> ::std::atomic<ListNode *> &;

  auto tail()
    -> ::std::atomic<ListNode *> &;

  auto alloc()
    -> ListNode *;

  void free(ListNode * node);

protected:

  // auto _acquireOrAllocate()
  //   -> ListNode *;

  // void _enqueueFreeList(ListNode * node);

  auto _tryDequeueFreeList()
    -> ListNode *;

  ::std::size_t                 _alloc_size;

  ::std::atomic<ListNode *>     _free_list_tail;
  ::std::atomic<ListNode *>     _head;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic<ListNode *>     _tail;
  ::std::atomic<ListNode *>     _free_list_head;

  ::std::atomic<StorageNode *>  _storage_head = { new StorageNode() };
  ::std::atomic<StorageNode *>  _storage_tail = { _storage_head.load(::std::memory_order_relaxed) };
};

}
}

#include "ustevent/core/concurrency/detail/ListNodeAllocator.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_LISTNODEALLOCATOR_H_
