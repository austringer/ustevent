#ifndef USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_
#define USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_

#include <functional>
#include "ustevent/core/detail/Operation.h"

namespace ustevent
{
namespace detail
{

template <typename Handler, typename Deallocator>
class ContextOperation : public Operation
{
public:
  template <typename H, typename D>
  ContextOperation(H && handler, D deallocator)
    : Operation(&ContextOperation::doPerform)
    , _handler(::std::forward<H>(handler))
    , _deallocator(deallocator)
  {}

  static
  void doPerform(Operation * base)
  {
    ContextOperation * op = static_cast<ContextOperation *>(base);
    Handler handler(::std::move(op->_handler));
    Deallocator deallocator(op->_deallocator);
    op->~ContextOperation();
    deallocator.deallocate(op);

    ::std::invoke(handler);
  }

private:
  Handler     _handler;
  Deallocator _deallocator;
};

}
}

#endif // USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_
