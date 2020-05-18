#ifndef USTEVENT_CORE_DETAIL_SPINMUTEX_H_
#define USTEVENT_CORE_DETAIL_SPINMUTEX_H_

#include <atomic>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include "ustevent/core/detail/Cache.h"

namespace ustevent
{
namespace detail
{

class SpinMutex
{
public:
  void lock();

  auto try_lock()
    -> bool;

  void unlock();

private:
  struct McsNode
  {
    USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
    volatile bool       _locked = true;

    USTEVENT_ALIGN_TO_AVOID_FALSE_SHARING
    volatile McsNode *  _next = nullptr;
  };

  ::std::atomic<McsNode *>  _tail = nullptr;

  static thread_local McsNode s_local_node;
};

static inline
void relax_cpu()
{
  #if defined(_MSC_VER)
    _mm_pause();
  #elif defined(__GNUC__) || defined(__clang__)
    asm volatile("pause\n": : :"memory");
  #endif
}

}
}

#endif // USTEVENT_CORE_DETAIL_SPINMUTEX_H_
