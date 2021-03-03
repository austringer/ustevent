#ifndef USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_
#define USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_

#include "ustevent/core/detail/ObjectPool.h"

namespace ustevent
{
namespace detail
{

template <typename T>
ObjectPool<T>::ObjectPool(::std::size_t next_chunk_size)
  : _pool(sizeof(T), next_chunk_size)
{}

template <typename T>
auto ObjectPool<T>::allocate()
  -> T *
{
  T * ret = static_cast<T*>(_pool.malloc());
  if (!ret)
  {
    throw std::bad_alloc();
  }
  return ret;
}

template <typename T>
void ObjectPool<T>::free(T * p)
{
  if (p)
  {
    _pool.free(p);
  }
}

}
}

#endif // USTEVENT_CORE_DETAIL_OBJECTPOOL_HPP_
