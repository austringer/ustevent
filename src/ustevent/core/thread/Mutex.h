#ifndef USTEVENT_CORE_THREAD_MUTEX_H_
#define USTEVENT_CORE_THREAD_MUTEX_H_

#include <mutex>

namespace ustevent
{
namespace thread
{

using Mutex                 = ::std::mutex;
using TimedMutex            = ::std::timed_mutex;
using RecursiveMutex        = ::std::recursive_mutex;
using RecursiveTimedMutex   = ::std::recursive_timed_mutex;

}
}

#endif // USTEVENT_CORE_THREAD_MUTEX_H_
