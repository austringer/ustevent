#ifndef USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_HPP_
#define USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_HPP_

#include "ustevent/core/concurrency/SpscListQueue.h"

namespace ustevent
{

template <typename T>
SpscListQueue<T>::SpscListQueue() = default;

template <typename T>
SpscListQueue<T>::~SpscListQueue() noexcept = default;

template <typename T>
void SpscListQueue<T>::enqueue(T const& input)
{
  _list_queue.enqueueSp(input);
}

template <typename T>
void SpscListQueue<T>::enqueue(T && input)
{
  _list_queue.enqueueSp(::std::move(input));
}

template <typename T>
auto SpscListQueue<T>::dequeue(T & output)
  -> bool
{
  return _list_queue.dequeueSc(output);
}

}

#endif // USTEVENT_CORE_CONCURRENCY_SPSCLISTQUEUE_HPP_
