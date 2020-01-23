#include "ustevent/core/Fiber.h"

namespace ustevent
{

Fiber::Fiber() = default;

Fiber::~Fiber() noexcept = default;

void Fiber::setPolicy(::boost::fibers::launch policy)
{
  _policy = policy;
}

void Fiber::setStackSize(::std::size_t stack_size)
{
  _stack_size = stack_size;
}

void Fiber::setDescription(::std::string description)
{
  _description = ::std::move(description);
}

void Fiber::join()
{
  assert(::boost::fibers::context::active()->get_id() != _context->get_id());
  _context->join();
  _context.reset();
}

void Fiber::detach()
{
  _context.reset();
}

Fiber::Fiber(Fiber && another) noexcept
{
  _context = ::std::move(another._context);
  _policy = another._policy;
  _stack_size = another._stack_size;
  _description = ::std::move(another._description);
}

auto Fiber::operator=(Fiber && another) noexcept
  -> Fiber &
{
  if (this != &another)
  {
    _context = ::std::move(another._context);
    _policy = another._policy;
    _stack_size = another._stack_size;
    _description = ::std::move(another._description);
  }
  return *this;
}

auto Fiber::_allocFiberDebugInfo(::boost::fibers::context * context)
  -> FiberDebugInfo *
{
  // The context will take the ownership of FiberDebugInfo instance.
  return new FiberDebugInfo(context);
}

}
