#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_H_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_H_

#include <atomic>
#include <mutex>
#include <vector>
#include <set>
#include <functional>

namespace ustevent
{
namespace detail
{

template <typename T, typename OWNER = void>
class ObjectScopedThreadLocalStorage
{
public:
  explicit
  ObjectScopedThreadLocalStorage(::std::function<T()> default_value_function = nullptr, ::std::function<void(T &&)> return_function = nullptr);

  ~ObjectScopedThreadLocalStorage() noexcept;

  auto get()
    -> T &;

  ObjectScopedThreadLocalStorage(ObjectScopedThreadLocalStorage const&) = delete;
  auto operator=(ObjectScopedThreadLocalStorage const&)
    -> ObjectScopedThreadLocalStorage & = delete;

private:
  struct Node
  {
    ::std::size_t               _owner_id = 0;
    T                           _value;
    ::std::function<void(T &&)> _return_function;
  };

  class ObjectStorage
  {
  public:
    ObjectStorage();

    ~ObjectStorage();

    auto get()
      -> ::std::vector<Node> &;

  private:
    ::std::vector<Node>  _values;
  };

  static
  auto _getIndex(::std::size_t my_id)
    -> ::std::size_t;

  auto _getThreadLocalObjectStorage()
    -> ::std::vector<Node> &;

  const ::std::function<T()>                _default_value_function;
  const ::std::function<void(T &&)>         _return_function;

  const ::std::size_t                       _my_id;
  const ::std::size_t                       _my_index;

  static ::std::atomic<::std::size_t>       s_id;

  static ::std::mutex                       s_mutex;
  static ::std::vector<::std::size_t>       s_available;      // vector of released box indexes
  static ::std::vector<::std::size_t>       s_owners;         // map box indexes to their owners id
  static ::std::set<ObjectStorage *>        s_thread_locals;  // global access to the thread local vectors
};

}
}

#include "ustevent/core/concurrency/detail/ObjectScopedThreadLocalStorage.hpp"

#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_H_
