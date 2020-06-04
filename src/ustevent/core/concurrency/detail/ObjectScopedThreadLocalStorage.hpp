#ifndef USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_HPP_
#define USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_HPP_

#include "ustevent/core/concurrency/detail/ObjectScopedThreadLocalStorage.h"

namespace ustevent
{
namespace detail
{

template <typename T, typename OWNER>
::std::atomic<::std::size_t> ObjectScopedThreadLocalStorage<T, OWNER>::s_id = { 1 };

template <typename T, typename OWNER>
::std::mutex ObjectScopedThreadLocalStorage<T, OWNER>::s_mutex;

template <typename T, typename OWNER>
::std::vector<::std::size_t> ObjectScopedThreadLocalStorage<T, OWNER>::s_available;

template <typename T, typename OWNER>
::std::vector<::std::size_t> ObjectScopedThreadLocalStorage<T, OWNER>::s_owners;

template <typename T, typename OWNER>
::std::set<typename ObjectScopedThreadLocalStorage<T, OWNER>::ObjectStorage *> ObjectScopedThreadLocalStorage<T, OWNER>::s_thread_locals;

template <typename T, typename OWNER>
ObjectScopedThreadLocalStorage<T, OWNER>::ObjectScopedThreadLocalStorage(::std::function<T()> default_value_function, ::std::function<void(T &&)> return_function)
  : _default_value_function(::std::move(default_value_function))
  , _return_function(::std::move(return_function))
  , _my_id(s_id++)
  , _my_index(_getIndex(_my_id))
{}

template <typename T, typename OWNER>
ObjectScopedThreadLocalStorage<T, OWNER>::~ObjectScopedThreadLocalStorage() noexcept
{
  ::std::scoped_lock<::std::mutex> lock(s_mutex);
  // invoke the returner for all thread local boxes corresponding to this index
  for (ObjectStorage * object_indexed_storage : s_thread_locals)
  {
    auto & v = object_indexed_storage->get();
    if (_my_index < v.size())
    {
      auto & storage = v[_my_index];
      // check if we've used that storage
      if (storage._owner_id == _my_id)
      {
        if (storage._return_function)
        {
          storage._return_function(::std::move(storage._value));
        }
        storage._owner_id = 0;
      }
    }
  }

  s_available.push_back(_my_index);
  s_owners[_my_index] = 0;
  if (s_available.size() == s_owners.size())
  {
    s_available.clear();
    s_owners.clear();
  }
}

template <typename T, typename OWNER>
auto ObjectScopedThreadLocalStorage<T, OWNER>::get()
  -> T &
{
  auto & v = _getThreadLocalObjectStorage();
  if (_my_index >= v.size())
  {
    // resize if accessed by a class with a higher counter
    ::std::scoped_lock<::std::mutex> lock(s_mutex);
    // check again now that we're sync'd
    if (_my_index >= v.size())
    {
      v.resize(_my_index + 1);
    }
  }

  // retrieve the item owned by this object instance
  auto & storage = v[_my_index];
  if (storage._owner_id == 0)
  {
    // if the owner is not set, need to apply the default value
    if (_default_value_function)
    {
      storage._value = _default_value_function();
    }
    storage._owner_id = _my_id;
    storage._return_function = _return_function;
  }
  return storage._value;
}

template <typename T, typename OWNER>
auto ObjectScopedThreadLocalStorage<T, OWNER>::_getIndex(::std::size_t my_id)
  -> ::std::size_t
{
  ::std::size_t index;
  ::std::scoped_lock<::std::mutex> lock(s_mutex);
  // check if a previous owner returned their index
  if (!s_available.empty())
  {
    // use that index if available
    index = s_available.back();
    s_available.pop_back();
    s_owners[index] = my_id;
  }
  else
  {
    // otherwise generate a new one
    index = s_owners.size();
    s_owners.push_back(my_id);
  }
  return index;
}

template <typename T, typename OWNER>
auto ObjectScopedThreadLocalStorage<T, OWNER>::_getThreadLocalObjectStorage()
  -> ::std::vector<typename ObjectScopedThreadLocalStorage<T, OWNER>::Node> &
{
  static thread_local ObjectStorage s;
  return s.get();
}

template <typename T, typename OWNER>
ObjectScopedThreadLocalStorage<T, OWNER>::ObjectStorage::ObjectStorage()
{
  ::std::scoped_lock<::std::mutex> lock(s_mutex);
  s_thread_locals.insert(this);
}

template <typename T, typename OWNER>
ObjectScopedThreadLocalStorage<T, OWNER>::ObjectStorage::~ObjectStorage()
{
  ::std::scoped_lock<::std::mutex> lock(s_mutex);
  for (::std::size_t i = 0; i < _values.size() && i < s_owners.size(); ++i)
  {
    auto & current_storage = _values[i];
    if (current_storage._return_function && current_storage._owner_id != 0 && s_owners[i] == current_storage._owner_id)
    {
      current_storage._return_function(::std::move(current_storage._value));
    }
  }
  s_thread_locals.erase(this);
}

template <typename T, typename OWNER>
auto ObjectScopedThreadLocalStorage<T, OWNER>::ObjectStorage::get()
  -> ::std::vector<typename ObjectScopedThreadLocalStorage<T, OWNER>::Node> &
{
  return _values;
}

}
}

#endif // USTEVENT_CORE_CONCURRENCY_DETAIL_OBJECTSCOPEDTHREADLOCALSTORAGE_HPP_
