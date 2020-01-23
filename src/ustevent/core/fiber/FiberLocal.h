#ifndef USTEVENT_CORE_FIBER_FIBERLOCAL_H_
#define USTEVENT_CORE_FIBER_FIBERLOCAL_H_

#include "boost/fiber/fss.hpp"

namespace ustevent
{

template <typename T>
using FiberLocal = ::boost::fibers::fiber_specific_ptr<T>;

}

#endif // USTEVENT_CORE_FIBER_FIBERLOCAL_H_
