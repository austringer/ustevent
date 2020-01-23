#ifndef USTEVENT_CORE_FIBER_BARRIER_H_
#define USTEVENT_CORE_FIBER_BARRIER_H_

#include "ustevent/core/detail/BasicBarrier.h"
#include "ustevent/core/fiber/Mutex.h"
#include "ustevent/core/fiber/ConditionVariable.h"

namespace ustevent
{
namespace fiber
{

using Barrier = ::ustevent::detail::BasicBarrier<::ustevent::fiber::Mutex, ::ustevent::fiber::ConditionVariable>;

}
}

#endif // USTEVENT_CORE_FIBER_BARRIER_H_
