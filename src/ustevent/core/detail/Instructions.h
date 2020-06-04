#ifndef USTEVENT_CORE_DETAIL_INSTRUCTIONS_H_
#define USTEVENT_CORE_DETAIL_INSTRUCTIONS_H_

namespace ustevent
{
namespace detail
{

static inline
void pause()
{
  #if defined(_MSC_VER)
    _mm_pause();
  #elif defined(__GNUC__) || defined(__clang__)
    asm volatile("pause\n": : :"memory");
  #endif
}

}
}

#endif // USTEVENT_CORE_DETAIL_INSTRUCTIONS_H_
