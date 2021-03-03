#ifndef USTEVENT_NET_LISTENER_H_
#define USTEVENT_NET_LISTENER_H_

#include <memory>

namespace ustevent
{
namespace net
{

class NetContext;
class Address;
class Connection;

class Listener
{
public:
  Listener() = default;

  virtual ~Listener() noexcept = default;

  virtual void setAcceptTimeout(int milliseconds) = 0;

  virtual void close() = 0;

  virtual auto accept()
    -> ::std::tuple<::std::unique_ptr<Connection>, int> = 0;

  virtual auto address()
    -> Address const* = 0;

  virtual auto getNetContext()
    -> NetContext & = 0;

  Listener(Listener const&) = delete;
  auto operator=(Listener const&)
    -> Listener & = delete;

  Listener(Listener &&) noexcept = default;
  auto operator=(Listener &&) noexcept
    -> Listener & = default;
};

}
}

#endif // USTEVENT_NET_LISTENER_H_
