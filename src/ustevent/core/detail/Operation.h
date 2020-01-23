#ifndef USTEVENT_CORE_DETAIL_OPERATION_H_
#define USTEVENT_CORE_DETAIL_OPERATION_H_

namespace ustevent
{
namespace detail
{

class Operation
{
public:
  ~Operation() noexcept = default;

  void perform()
  {
    _func(this);
  }

protected:
  using Func = void(Operation *);

  Operation(Func func)
    : _func(func)
  {}

private:
  Func * _func;
};

}
}

#endif // USTEVENT_CORE_DETAIL_OPERATION_H_
