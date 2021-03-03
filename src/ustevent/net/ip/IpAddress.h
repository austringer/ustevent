#ifndef USTEVENT_NET_IP_IPADDRESS_H_
#define USTEVENT_NET_IP_IPADDRESS_H_

#include <array>
#include <memory>
#include "ustevent/net/Address.h"

namespace ustevent
{
namespace net
{

enum
{
  SIZE_OF_IPV4ADDRESS = 4,
  SIZE_OF_IPV6ADDRESS = 16,
};

class IpV4Address;
class IpV6Address;

class IpAddress : public Address
{
public:
  static auto parse(::std::string_view const host)
    -> ::std::unique_ptr<IpAddress>;

  static auto parse(::std::string_view const host, void * address, ::std::size_t address_size)
    -> bool;
};

class IpV4Address : public IpAddress
{
public:

  static auto parse(::std::string_view const host)
    -> ::std::unique_ptr<IpV4Address>;

  static auto parse(::std::string_view const host, void * address, ::std::size_t address_size)
    -> bool;

  IpV4Address();

  ~IpV4Address() noexcept override;

  IpV4Address(IpV4Address const& rhs);

  auto operator=(IpV4Address const& rhs)
    -> IpV4Address &;

  auto protocal() const
    -> Protocal override;

  auto data() const
    -> void const* override;

  auto data()
    -> void * override;

  auto size() const
    -> ::std::size_t override;

  auto string() const
    -> ::std::string_view override;

  auto toV6() const
    -> ::std::unique_ptr<IpV6Address>;

  auto toV6(void * address, ::std::size_t address_size) const
    -> bool;

  friend auto operator==(IpV4Address const& lhs, IpV4Address const& rhs)
    -> bool;

  friend auto operator==(IpV4Address const& lhs, IpV6Address const& rhs)
    -> bool;

private:
  const Protocal            _protocal = IPV4;

  using IpAddressDataV4 = ::std::array<uint8_t, SIZE_OF_IPV4ADDRESS>;
  IpAddressDataV4           _address_v4;

  friend class IpV6Address;
};

class IpV6Address : public IpAddress
{
public:

  static auto parse(::std::string_view const host)
    -> ::std::unique_ptr<IpV6Address>;

  static auto parse(::std::string_view const host, void * address, ::std::size_t address_size)
    -> bool;

  IpV6Address();

  ~IpV6Address() noexcept override;

  IpV6Address(IpV6Address const& rhs);

  auto operator=(IpV6Address const& rhs)
    -> IpV6Address &;

  auto protocal() const
    -> Protocal override;

  auto data() const
    -> void const* override;

  auto data()
    -> void * override;

  auto size() const
    -> ::std::size_t override;

  auto string() const
    -> ::std::string_view override;

  auto toV4() const
    -> ::std::unique_ptr<IpV4Address>;

  auto toV4(void * address, ::std::size_t address_size) const
    -> bool;

  friend auto operator==(IpV6Address const& lhs, IpV6Address const& rhs)
    -> bool;

  friend auto operator==(IpV6Address const& lhs, IpV4Address const& rhs)
    -> bool;

private:
  const Protocal            _protocal = IPV6;

  using IpAddressDataV6 = ::std::array<uint8_t, SIZE_OF_IPV6ADDRESS>;
  IpAddressDataV6           _address_v6;

  friend class IpV4Address;
};

}
}

#endif // USTEVENT_NET_IP_IPADDRESS_H_
