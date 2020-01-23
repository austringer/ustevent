#ifndef USTEVENT_NET_CONNECTION_H_
#define USTEVENT_NET_CONNECTION_H_

#include <memory>
#include "ustevent/net/Address.h"

namespace ustevent
{
namespace net
{

class Address;

class Connection
{
public:
  Connection() = default;

  virtual ~Connection() noexcept = default;

  virtual void close() = 0;

  virtual void interrupt() = 0;

  virtual void setReadTimeout(int milliseconds) = 0;

  virtual void setWriteTimeout(int milliseconds) = 0;

  // return: bytes received, error
  virtual auto readSome(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> = 0;

  // return: bytes_sent, error
  virtual auto writeSome(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> = 0;

  // return: bytes received, error
  virtual auto read(void * buffer, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> = 0;

  // return: bytes_sent, error
  virtual auto write(void const* data, ::std::size_t size)
    -> ::std::tuple<::std::size_t, int> = 0;

  virtual auto localAddress() const
    -> Address const* = 0;

  virtual auto remoteAddress() const
    -> Address const* = 0;

  Connection(Connection const&) = delete;
  auto operator=(Connection const&)
    -> Connection & = delete;

  Connection(Connection &&) noexcept = default;
  auto operator=(Connection &&) noexcept
    -> Connection & = default;

};

}
}

#endif // USTEVENT_NET_CONNECTION_H_
