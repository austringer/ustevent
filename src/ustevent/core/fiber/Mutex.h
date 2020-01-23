#ifndef USTEVENT_CORE_FIBER_MUTEX_H_
#define USTEVENT_CORE_FIBER_MUTEX_H_

#include "boost/fiber/mutex.hpp"
#include "boost/fiber/timed_mutex.hpp"
#include "boost/fiber/recursive_mutex.hpp"
#include "boost/fiber/recursive_timed_mutex.hpp"

namespace ustevent
{
namespace fiber
{

using Mutex                 = ::boost::fibers::mutex;
using TimedMutex            = ::boost::fibers::timed_mutex;
using RecursiveMutex        = ::boost::fibers::recursive_mutex;
using RecursiveTimedMutex   = ::boost::fibers::recursive_timed_mutex;

}
}

#endif // USTEVENT_CORE_FIBER_MUTEX_H_
