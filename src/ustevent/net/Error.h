#ifndef USTEVENT_NET_ERROR_H_
#define USTEVENT_NET_ERROR_H_

namespace ustevent
{
namespace net
{

struct Error
{
  enum ErrorNum
  {
    CLOSED                        = -1,
    REMOTELY_CLOSED               = -2,
    TIMEOUT                       = -3,
    INTERRUPTED                   = -4,
    INVALID_ARGUMENT              = -5,
    INVALID_NETWORK_PROTOCAL      = -6,
    INVALID_TRANSPORT_PROTOCAL    = -7,
  };
};

}
}

#endif // USTEVENT_NET_ERROR_H_
