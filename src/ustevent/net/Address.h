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
    -> Protocal;

  virtual auto string() const
    -> ::std::string_view;
};

}
}

#endif // USTEVENT_NET_ADDRESS_H_
