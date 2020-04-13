#ifndef USTEVENT_CORE_DETAIL_BASICBARRIER_H_
#define USTEVENT_CORE_DETAIL_BASICBARRIER_H_

#include <cstddef>

namespace ustevent
{
namespace detail
{

template <typename Mutex, typename ConditionVariable>
class BasicBarrier
{
public:
  explicit
  BasicBarrier(::std::size_t count) noexcept;

  auto wait()
    -> bool;

  BasicBarrier(BasicBarrier const&) = delete;
  auto operator=(BasicBarrier const&)
    -> BasicBarrier & = delete;

private:
  mutable Mutex                       _mutex;
  mutable ConditionVariable           _cv;
  ::std::size_t const                 _threshold;
  ::std::size_t                       _count;
  ::std::size_t                       _generation;
};

}
}

#include "ustevent/core/detail/BasicBarrier.hpp"

#endif // USTEVENT_CORE_DETAIL_BASICBARRIER_H_
