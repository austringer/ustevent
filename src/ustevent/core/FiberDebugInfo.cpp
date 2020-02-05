#include "ustevent/core/FiberDebugInfo.h"

// #include <execinfo.h> // backtrace
#include <cassert>
#include "ustevent/core/detail/Macros.h"

namespace ustevent
{

FiberDebugInfo::FiberDebugInfo(::boost::fibers::context * context)
  : ::boost::fibers::fiber_properties(context)
  // , _backtrace()
{}

void FiberDebugInfo::updateFrame()
{
  // _size = ::backtrace(_backtrace, USTEVENT_ARRAY_SIZE(_backtrace));
  unw_getcontext(&_unwind_context);
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


void ustevent_backtraceFiber(
  unw_context_t * context,
  uintptr_t *     ip_stack,
  size_t          ip_stack_length,
  size_t *        frame_depth)
{
  assert(context != nullptr);
  assert(ip_stack != nullptr);
  assert(frame_depth != nullptr);

  unw_cursor_t cursor;
  unw_init_local(&cursor, context);

  size_t i = 0;
  while (i < ip_stack_length && unw_step(&cursor))
  {
    auto ip = reinterpret_cast<unw_word_t *>(ip_stack + i);
    unw_get_reg(&cursor, UNW_REG_IP, ip);
    ++i;
  }
  *frame_depth = i;
}

// void ustevent_backtrace(unw_context_t * unwind_context)
// {
//   unw_cursor_t cursor;
//   unw_init_local(&cursor, unwind_context);

//   int n = 0;
//   while (unw_step(&cursor))
//   {
//     unw_word_t ip, sp, off;
//     unw_get_reg(&cursor, UNW_REG_IP, &ip);
//     unw_get_reg(&cursor, UNW_REG_SP, &sp);
//   }
// }
