#ifndef USTEVENT_CORE_FIBERDEBUGINFO_H_
#define USTEVENT_CORE_FIBERDEBUGINFO_H_

#include <string>
#include "boost/fiber/properties.hpp"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace ustevent
{

struct FiberDebugInfo : public ::boost::fibers::fiber_properties
{
public:
  explicit
  FiberDebugInfo(::boost::fibers::context * context);

  void updateFrame();

  void setDescription(::std::string description);

  void setTerminated(bool terminated);

  bool terminated() const;

  FiberDebugInfo *  _prev = nullptr;
  FiberDebugInfo *  _next = nullptr;

  FiberDebugInfo(FiberDebugInfo const&) = delete;
  auto operator=(FiberDebugInfo const&)
    -> FiberDebugInfo & = delete;

private:

  ::std::string     _description;
  bool              _terminated = false;

  int               _size = 0;
  void *            _backtrace[64];

  unw_context_t     _unwind_context;
};

}

#ifdef __cplusplus
extern "C" {
#endif

void ustevent_backtraceFiber(
  unw_context_t * unwind_context,
  uintptr_t *     ip_stack,
  size_t          ip_stack_length,
  size_t *        frame_depth);

// void ustevent_backtrace(unw_context_t * context);

#ifdef __cplusplus
}
#endif

#endif // USTEVENT_CORE_FIBERDEBUGINFO_H_
