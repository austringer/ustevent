#ifndef USTEVENT_CORE_FIBERDEBUGINFO_H_
#define USTEVENT_CORE_FIBERDEBUGINFO_H_

#include <string>
#include "boost/fiber/properties.hpp"

namespace ustevent
{

struct FiberDebugInfo : public ::boost::fibers::fiber_properties
{
public:
  explicit
  FiberDebugInfo(::boost::fibers::context * context);

  void backtrace();

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
};

}

#endif // USTEVENT_CORE_FIBERDEBUGINFO_H_
