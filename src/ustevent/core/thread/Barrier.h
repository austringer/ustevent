#ifndef USTEVENT_CORE_THREAD_BARRIER_H_
#define USTEVENT_CORE_THREAD_BARRIER_H_

#include "ustevent/core/detail/BasicBarrier.h"
#include "ustevent/core/thread/Mutex.h"
#include "ustevent/core/thread/ConditionVariable.h"

namespace ustevent
{
namespace thread
{

using Barrier = ::ustevent::detail::BasicBarrier<::ustevent::thread::Mutex, ::ustevent::thread::ConditionVariable>;

}
}

#endif // USTEVENT_CORE_THREAD_BARRIER_H_
