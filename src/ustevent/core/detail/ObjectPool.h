#ifndef USTEVENT_CORE_DETAIL_OBJECTPOOL_H_
#define USTEVENT_CORE_DETAIL_OBJECTPOOL_H_

#include <atomic>
#include <vector>
#include "ustevent/core/detail/Cache.h"

namespace ustevent
{
namespace detail
{

template <typename T, ::std::size_t Alloc = 32>
class ObjectPool
{
public:
  ObjectPool();

  ~ObjectPool() noexcept;

  auto alloc()
    -> T *;

  void free(T * object);

  ObjectPool(ObjectPool const&) = delete;
  auto operator=(ObjectPool const&)
    -> ObjectPool & = delete;

private:
  struct ListNode
  {
    T                             _data;

    USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
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
  }

  auto _tryDequeueFreeList()
    -> ListNode *;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic<ListNode *>     _free_list_tail;

  USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
  ::std::atomic<ListNode *>     _free_list_head;

  ::std::atomic<StorageNode *>  _storage_head;
  ::std::atomic<StorageNode *>  _storage_tail;;
};

}
}

#include "ustevent/core/detail/ObjectPool.hpp"

#endif // USTEVENT_CORE_DETAIL_OBJECTPOOL_H_
