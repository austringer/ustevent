#ifndef USTEVENT_CORE_FIBER_SEMAPHORE_H_
#define USTEVENT_CORE_FIBER_SEMAPHORE_H_

#include "ustevent/core/detail/BasicSemaphore.h"
#include "ustevent/core/fiber/Mutex.h"
#include "ustevent/core/fiber/ConditionVariable.h"

namespace ustevent
{
namespace fiber
{

using Semaphore = ::ustevent::detail::BasicSemaphore<::ustevent::fiber::Mutex, ::ustevent::fiber::ConditionVariable>;

}
}

#endif // USTEVENT_CORE_FIBER_SEMAPHORE_H_
