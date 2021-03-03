#ifndef USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_HPP_
#define USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_HPP_

#include "ustevent/core/detail/ContextTaskQueue.h"
#include <cstring>
#include <thread>

namespace ustevent
{
namespace detail
{

template <typename Op>
void ContextTaskQueue::push(Op && operation, ::std::size_t stack_size, ::std::string_view description)
{
  ::std::ptrdiff_t const operation_length_offset    = sizeof(::std::uint32_t);
  ::std::ptrdiff_t const operation_offset           = operation_length_offset + sizeof(::std::uint32_t);
  ::std::ptrdiff_t const stack_size_offset          = operation_offset + _roundUp(sizeof(operation));
  ::std::ptrdiff_t const description_length_offset  = stack_size_offset + sizeof(::std::uint32_t);
  ::std::ptrdiff_t const description_offset         = description_length_offset + sizeof(::std::uint32_t);

  ::std::size_t const acquired_size = static_cast<::std::size_t>(description_offset + _roundUp(description.size() + 1));

  auto & writer = _writers.get();
  void * buffer;
  do
  {
    if (!writer.acquire(acquired_size, &buffer))
    {
      ::std::this_thread::yield();
      continue;
    }

    unsigned char * base = static_cast<unsigned char *>(buffer);

    ::std::uint32_t * context_task_length_p = static_cast<::std::uint32_t *>(static_cast<void *>(base));
    *(context_task_length_p) = static_cast<::std::uint32_t>(acquired_size);

    ::std::uint32_t * operation_length_p = static_cast<::std::uint32_t *>(static_cast<void *>(base + operation_length_offset));
    *(operation_length_p) = static_cast<::std::uint32_t>(sizeof(operation));

    using OpType = ::std::decay_t<Op>;
    OpType * context_operation_p = static_cast<OpType *>(static_cast<void *>(base + operation_offset));
    new (context_operation_p) OpType(::std::forward<Op>(operation));

    ::std::uint32_t * stack_size_p = static_cast<::std::uint32_t *>(static_cast<void *>(base + stack_size_offset));
    *(stack_size_p) = static_cast<::std::uint32_t>(stack_size);

    ::std::uint32_t * description_length_p = static_cast<::std::uint32_t *>(static_cast<void *>(base + description_length_offset));
    *(description_length_p) = static_cast<::std::uint32_t>(description.size());

    char * description_p = static_cast<char *>(static_cast<void *>(base + description_offset));
    ::std::memcpy(description_p, description.data(), description.size());
    description_p[description.size()] = '\0';

    writer.produce();
    return;
  } while (true);
}

template <typename Task>
void ContextTaskQueue::TaskDeallocator::deallocate(Task * address)
{
  ::std::ptrdiff_t const constext_task_length_offset = 0 - sizeof(::std::uint32_t) - sizeof(::std::uint32_t);
  unsigned char * base = static_cast<unsigned char *>(static_cast<void *>(address)) + constext_task_length_offset;
  ::std::uint32_t * context_task_length = static_cast<::std::uint32_t *>(static_cast<void *>(base));
  _buffer_reader->release(*context_task_length);
}

constexpr
auto ContextTaskQueue::_roundUp(::std::size_t num)
  -> ::std::size_t
{
  return (num + sizeof(::std::size_t) - 1) & -sizeof(::std::size_t);
}

}
}

#endif // USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_HPP_
