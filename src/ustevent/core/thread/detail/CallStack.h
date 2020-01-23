#ifndef USTEVENT_CORE_THREAD_DETAIL_CALLSTACK_H_
#define USTEVENT_CORE_THREAD_DETAIL_CALLSTACK_H_

#include <cstdint>

namespace ustevent
{
namespace thread
{

template <typename Key, typename Value = ::std::uint8_t>
class CallStack
{
public:
  class Context
  {
  public:
    Context(Key * key, Value & value)
      : _next(CallStack<Key, Value>::_s_top)
      , _key(key)
      , _value(&value)
    {
      CallStack<Key, Value>::_s_top = this;
    }

    explicit
    Context(Key * key)
    : _next(CallStack<Key, Value>::_s_top)
    , _key(key)
    , _value(reinterpret_cast<::std::uint8_t *>(this))
  {
    CallStack<Key, Value>::_s_top = this;
  }

    Context(Context const&) = delete;
    Context & operator=(Context const&) = delete;

    friend class CallStack;
  private:
    Context * _next;
    Key *     _key;
    Value *   _value;
  };

  static auto contain(Key const* key)
    -> Value *
  {
    Context * element = _s_top;
    while (element)
    {
      if (element->_key == key)
      {
        return element->_value;
      }
      element = element->_next;
    }
    return nullptr;
  }

private:
  static thread_local Context * _s_top;
};

template <typename Key, typename Value>
thread_local typename CallStack<Key, Value>::Context * CallStack<Key, Value>::_s_top = nullptr;

}
}

#endif // USTEVENT_CORE_THREAD_DETAIL_CALLSTACK_H_
