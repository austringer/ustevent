#ifndef USTEVENT_NET_DIALER_H_
#define USTEVENT_NET_DIALER_H_

#include <memory>
#include <tuple>

namespace ustevent
{
namespace net
{

class Address;
class Connection;

class Dialer
{
public:
  Dialer() = default;

  virtual ~Dialer() noexcept = default;

  virtual void setConnectTimeout(int milliseconds) = 0;

  virtual auto connect(Address const& address)
    -> ::std::tuple<::std::unique_ptr<Connection>, int> = 0;

  virtual void interrupt() = 0;

  Dialer(Dialer const&) = delete;
  auto operator=(Dialer const&)
    -> Dialer & = delete;

  Dialer(Dialer &&) noexcept = default;
  auto operator=(Dialer &&) noexcept
    -> Dialer & = default;
};


}
}

#endif // USTEVENT_NET_DIALER_H_
