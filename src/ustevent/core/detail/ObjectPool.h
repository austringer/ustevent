#ifndef USTEVENT_CORE_DETAIL_OBJECTPOOL_H_
#define USTEVENT_CORE_DETAIL_OBJECTPOOL_H_

#include <cstddef>
#include <boost/pool/pool.hpp>

namespace ustevent
{
namespace detail
{

template <typename T>
class ObjectPool
{
public:
  ObjectPool(::std::size_t next_chunk_size = 32);

  auto allocate()
    -> T *;

  void free(T * p)
  {
      if (p)
      {
          _pool.free(p);
      }
  }

private:
  boost::pool<boost::default_user_allocator_malloc_free> _pool;
};

}
}

#endif // USTEVENT_CORE_DETAIL_OBJECTPOOL_H_
