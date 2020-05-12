#ifndef USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_HPP_
#define USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_HPP_

#include "ustevent/core/concurrency/SpmcListQueue.h"

namespace ustevent
{

template <typename T>
SpmcListQueue<T>::SpmcListQueue() = default;

template <typename T>
SpmcListQueue<T>::~SpmcListQueue() noexcept = default;

template <typename T>
void SpmcListQueue<T>::enqueue(T const& input)
{
  _list_queue.enqueueSp(input);
}

template <typename T>
void SpmcListQueue<T>::enqueue(T && input)
{
  _list_queue.enqueueSp(::std::move(input));
}

template <typename T>
auto SpmcListQueue<T>::dequeue(T & output)
  -> bool
{
  return _list_queue.dequeueMc(output);
}

template <typename T>
auto SpmcListQueue<T>::dequeueWeak(T & output)
  -> bool
{
  return _list_queue.dequeueMcWeak(output);
}

}

#endif // USTEVENT_CORE_CONCURRENCY_SPMCLISTQUEUE_HPP_
