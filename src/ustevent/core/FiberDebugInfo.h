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

  ::std::size_t     _size = 0;
  unw_word_t        _backtrace[64];
};

}

#endif // USTEVENT_CORE_FIBERDEBUGINFO_H_
