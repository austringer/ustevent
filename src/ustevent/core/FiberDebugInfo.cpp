#include "ustevent/core/FiberDebugInfo.h"

#include <cassert>
#include "ustevent/core/detail/Macros.h"

namespace ustevent
{

FiberDebugInfo::FiberDebugInfo(::boost::fibers::context * context)
  : ::boost::fibers::fiber_properties(context)
  , _backtrace()
{}

void FiberDebugInfo::updateFrame()
{
  // _size = ::backtrace(_backtrace, USTEVENT_ARRAY_SIZE(_backtrace));
  constexpr auto backtrace_size = USTEVENT_ARRAY_SIZE(_backtrace);
  unw_context_t context;
  unw_cursor_t cursor;
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);
  ::std::size_t i = 0;
  do
  {
    unw_get_reg(&cursor, UNW_REG_IP, _backtrace + i++);
    if (unw_step(&cursor) == 0)
    {
      if (i < backtrace_size)
      {
        unw_get_reg(&cursor, UNW_REG_IP, _backtrace + i++);
      }
      break;
    }
  } while (i < backtrace_size);

  _size = i;

  for (;i < backtrace_size; ++i)
  {
    _backtrace[i] = 0;
  }
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
