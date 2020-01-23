#include "ustevent/core/FiberDebugInfo.h"

#include <execinfo.h> // backtrace
#include "ustevent/core/detail/Macros.h"

namespace ustevent
{

FiberDebugInfo::FiberDebugInfo(::boost::fibers::context * context)
  : ::boost::fibers::fiber_properties(context)
  , _backtrace()
{}

void FiberDebugInfo::backtrace()
{
  _size = ::backtrace(_backtrace, USTEVENT_ARRAY_SIZE(_backtrace));
}

void FiberDebugInfo::setDescription(::std::string description)
{
  _description = ::std::move(description);
}

void FiberDebugInfo::setTerminated(bool terminated)
{
  _terminated = terminated;
}

bool FiberDebugInfo::terminated() const
{
  return _terminated;
}

}
