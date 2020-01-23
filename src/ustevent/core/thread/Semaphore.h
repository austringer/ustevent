#ifndef USTEVENT_CORE_THREAD_SEMAPHORE_H_
#define USTEVENT_CORE_THREAD_SEMAPHORE_H_

#include "ustevent/core/detail/BasicSemaphore.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/thread/ConditionVariable.h"

namespace ustevent
{
namespace thread
{

using Semaphore = ::ustevent::detail::BasicSemaphore<::ustevent::thread::Mutex, ::ustevent::thread::ConditionVariable>;

}
}

#endif // USTEVENT_CORE_THREAD_SEMAPHORE_H_
