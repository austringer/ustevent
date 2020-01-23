#ifndef USTEVENT_NET_IP_IPADDRESS_H_
#define USTEVENT_NET_IP_IPADDRESS_H_

#include <array>
#include <memory>
#include "ustevent/net/Address.h"

namespace ustevent
{
namespace net
{

class IpAddress : public Address
{
public:
  struct V4 {};
  struct V6 {};

  static const V4 v4;
  static const V6 v6;

  static auto parse(::std::string_view const host)
    -> ::std::unique_ptr<IpAddress>;

  template <typename IPVersion>
  static auto parse(::std::string_view const host)
    -> ::std::unique_ptr<IpAddress>;

  IpAddress();

  explicit
  IpAddress(V4);

  explicit
  IpAddress(V6);

  ~IpAddress() noexcept override;

  auto protocal() const
    -> Protocal override;
  auto string() const
    -> ::std::string_view override;

  auto data()
    -> void *;
  auto data() const
    -> void const*;

  auto size() const
    -> ::std::size_t;

  friend auto operator==(IpAddress const& lhs, IpAddress const& rhs)
    -> bool;

private:

  Protocal          _version;

  using IPAddressDataV4 = ::std::array<uint8_t, 4>;
  using IPAddressDataV6 = ::std::array<uint8_t, 16>;
  union
  {
    IPAddressDataV4 _address_v4;
    IPAddressDataV6 _address_v6;
  };


};

}
}

#endif // USTEVENT_NET_IP_IPADDRESS_H_
