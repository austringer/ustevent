#ifndef USTEVENT_CORE_FIBER_DETAIL_CALLSTACK_H_
#define USTEVENT_CORE_FIBER_DETAIL_CALLSTACK_H_

#include <cstdint>
#include "ustevent/core/fiber/FiberLocal.h"

namespace ustevent
{
namespace fiber
{

template <typename Key, typename Value = uint8_t>
class CallStack
{
public:
  class Context
  {
  public:
    Context(Key * key, Value & value)
      : _key(key)
      , _value(&value)
    {
      Context* * previous_top = CallStack<Key, Value>::_s_top.get();
      if (previous_top)
      {
        _next = *previous_top;
      }
      else
      {
        _next = nullptr;
      }

      CallStack<Key, Value>::_s_top.reset(new (Context *)(this));
    }

    explicit
    Context(Key * key)
      : _key(key)
      , _value(reinterpret_cast<std::uint8_t *>(this))
    {
      Context* * previous_top = CallStack<Key, Value>::_s_top.get();
      if (previous_top)
      {
        _next = *previous_top;
      }
      else
      {
        _next = nullptr;
      }

      CallStack<Key, Value>::_s_top.reset(new (Context *)(this));
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
    Context* * element_pointer = _s_top.get();
    if(element_pointer)
    {
      Context * element = *element_pointer;
      while (element)
      {
        if (element->_key == key)
        {
          return element->_value;
        }
        element = element->_next;
      }
    }
    return nullptr;
  }

private:
  static FiberLocal<Context *> _s_top;
};

template <typename Key, typename Value>
FiberLocal<typename CallStack<Key, Value>::Context *> CallStack<Key, Value>::_s_top =
  FiberLocal<typename CallStack<Key, Value>::Context *>();

}
}

#endif // USTEVENT_CORE_FIBER_DETAIL_CALLSTACK_H_
