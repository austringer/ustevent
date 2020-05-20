#ifndef USTEVENT_NET_NETCONTEXT_H_
#define USTEVENT_NET_NETCONTEXT_H_

#include <memory>
#include "ustevent/core/Context.h"
#include "ustevent/net/detail/EventSelector.h"

namespace ustevent
{
namespace net
{

class NetContext : public Context
{
public:
  NetContext();

  ~NetContext() noexcept;

  void run();

  auto selector()
    -> detail::EventSelector &;

private:
  ::std::unique_ptr<detail::EventSelector> _event_selector;

  friend class MultiplexingContextStrategy;
};

}
}

#endif // USTEVENT_NET_NETCONTEXT_H_
