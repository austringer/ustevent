#ifndef USTEVENT_CORE_FIBER_CONDITIONVARIABLE_H_
#define USTEVENT_CORE_FIBER_CONDITIONVARIABLE_H_

#include "boost/fiber/condition_variable.hpp"

namespace ustevent
{
namespace fiber
{

using ConditionVariable     = ::boost::fibers::condition_variable;
using ConditionVariableAny  = ::boost::fibers::condition_variable_any;

}
}

#endif // USTEVENT_CORE_FIBER_CONDITIONVARIABLE_H_
