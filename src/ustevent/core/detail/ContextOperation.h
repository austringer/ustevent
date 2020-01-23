#ifndef USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_
#define USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_

#include "ustevent/core/detail/Operation.h"

namespace ustevent
{
namespace detail
{

template <typename Handler>
class ContextOperation : public Operation
{
public:
  template <typename H>
  ContextOperation(H && handler)
    : Operation(&ContextOperation::doPerform)
    , _handler(::std::forward<H>(handler))
  {}

  static
  void doPerform(Operation * base)
  {
    ContextOperation * op = static_cast<ContextOperation *>(base);
    ::std::invoke(op->_handler);
  }

private:
  Handler _handler;
};

}
}

#endif // USTEVENT_CORE_DETAIL_CONTEXTOPERATION_H_
