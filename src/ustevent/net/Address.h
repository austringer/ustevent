#ifndef USTEVENT_NET_ADDRESS_H_
#define USTEVENT_NET_ADDRESS_H_

#include <string>
#include "ustevent/net/Protocal.h"

namespace ustevent
{
namespace net
{

class Address
{
public:
  virtual ~Address() noexcept = default;

  virtual auto protocal() const
    -> Protocal = 0;

  virtual auto data() const
    -> void const* = 0;

  virtual auto data()
    -> void * = 0;

  virtual auto size() const
    -> ::std::size_t = 0;

  virtual auto string() const
    -> ::std::string_view = 0;
};

}
}

#endif // USTEVENT_NET_ADDRESS_H_
