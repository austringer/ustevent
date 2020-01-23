#ifndef USTEVENT_CORE_THREAD_CONDITIONVARIABLE_H_
#define USTEVENT_CORE_THREAD_CONDITIONVARIABLE_H_

#include <condition_variable>

namespace ustevent
{
namespace thread
{

using ConditionVariable     = ::std::condition_variable;
using ConditionVariableAny  = ::std::condition_variable_any;

}
}

#endif // USTEVENT_CORE_THREAD_CONDITIONVARIABLE_H_
