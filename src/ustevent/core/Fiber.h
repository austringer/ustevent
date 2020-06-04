#ifndef USTEVENT_CORE_FIBER_H_
#define USTEVENT_CORE_FIBER_H_

#include <string>
#include "boost/fiber/context.hpp"

namespace ustevent
{

class FiberDebugInfo;

class Fiber final
{
public:
  Fiber();

  ~Fiber() noexcept;

  void setPolicy(::boost::fibers::launch policy);

  void setStackSize(::std::size_t stack_size);

  void setDescription(::std::string description);

  void setDescription(::std::string_view description);

  template <typename Fn, typename ... Args>
  void start(Fn && function, Args ... args);

  void join();

  void detach();

  Fiber(Fiber && another) noexcept;
  auto operator=(Fiber && another) noexcept
    -> Fiber &;

  Fiber(Fiber const&) = delete;
  auto operator=(Fiber const&)
    -> Fiber & = delete;

private:
  using ContextPtr          = ::boost::intrusive_ptr<::boost::fibers::context>;
  ContextPtr                  _context;

  ::boost::fibers::launch     _policy = ::boost::fibers::launch::post;

  ::std::size_t               _stack_size = 0;

  ::std::string               _description;

  auto _allocFiberDebugInfo(::boost::fibers::context * context)
    -> FiberDebugInfo *;
};

}

#include "ustevent/core/Fiber.hpp"

#endif // USTEVENT_CORE_FIBER_H_
