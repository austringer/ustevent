#ifndef USTEVENT_NET_MULTIPLEXINGCONTEXTSTRATEGY_H_
#define USTEVENT_NET_MULTIPLEXINGCONTEXTSTRATEGY_H_

#include "ustevent/core/ContextStrategy.h"

namespace ustevent
{
namespace net
{
namespace detail
{
class EventSelector;
}

class NetContext;

class MultiplexingContextStrategy : public ContextStrategy
{
public:
  explicit
  MultiplexingContextStrategy(NetContext & net_context);

  enum BlockingFlag
  {
    BLOCKING,
    POLLING,
  };

  MultiplexingContextStrategy(NetContext & net_context, BlockingFlag blocking_flag);

  ~MultiplexingContextStrategy() noexcept override;

  MultiplexingContextStrategy(MultiplexingContextStrategy const&) = delete;
  auto operator=(MultiplexingContextStrategy const&)
    -> MultiplexingContextStrategy & = delete;

protected:
  void onSuspended(::std::chrono::steady_clock::time_point const& time_point) noexcept override;

  void onNotified() noexcept override;

private:
  BlockingFlag                      _blocking_flag;

  detail::EventSelector &           _event_selector;

  ::std::thread::native_handle_type _this_thread;
};

}
}

#endif // USTEVENT_NET_MULTIPLEXINGCONTEXTSTRATEGY_H_
