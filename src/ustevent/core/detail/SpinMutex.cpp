#include "ustevent/core/detail/SpinMutex.h"

namespace ustevent
{
namespace detail
{

thread_local SpinMutex::McsNode SpinMutex::s_local_node = {};

void SpinMutex::lock()
{
  auto prev_node = _tail.exchange(&s_local_node, ::std::memory_order_acquire);
  if (prev_node != nullptr)
  {
    s_local_node._locked = true;
    prev_node->_next = &s_local_node;
    while (s_local_node._locked)
    {
      relax_cpu();
    }
  }
}

auto SpinMutex::try_lock()
  -> bool
{
  McsNode * p = nullptr;
  return _tail.compare_exchange_strong(
    p, &s_local_node,
    ::std::memory_order_acquire,
    ::std::memory_order_relaxed);
}

void SpinMutex::unlock()
{
  if(s_local_node._next == nullptr)
  {
    McsNode * p = &s_local_node;
    if (_tail.compare_exchange_strong(
      p, nullptr,
      ::std::memory_order_release,
      ::std::memory_order_relaxed))
    {
      return;
    }
    while (s_local_node._next == nullptr) {}
    s_local_node._next->_locked = false;
    s_local_node._next = nullptr;
  }
}

}
}
