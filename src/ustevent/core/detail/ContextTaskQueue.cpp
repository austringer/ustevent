#include "ustevent/core/detail/ContextTaskQueue.h"

#include <thread>

namespace ustevent
{
namespace detail
{

ContextTaskQueue::ContextTaskQueue(::std::size_t buffer_length)
  : _ring_buffer(buffer_length, ::std::thread::hardware_concurrency())
  , _writers(
    [this]()
    {
      return MpscRingBufferWriter(_ring_buffer, _writer_sequence.fetch_add(1, ::std::memory_order_acq_rel));
    },
    [this](MpscRingBufferWriter && writer)
    {
      writer.~MpscRingBufferWriter();
    })
  , _reader(_ring_buffer)
{}

auto ContextTaskQueue::pop(Operation * * operation, ::std::size_t * stack_size, ::std::string_view * description)
  -> bool
{
  if (_poped < _consumed)
  {
    unsigned char * base = _poped;

    ::std::uint32_t context_task_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base)));

    ::std::ptrdiff_t const operation_length_offset = sizeof(::std::uint32_t);
    ::std::uint32_t operation_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + operation_length_offset)));
    ::std::ptrdiff_t const operation_offset = operation_length_offset + sizeof(::std::uint32_t);
    if (operation != nullptr)
    {
      *operation = static_cast<Operation *>(static_cast<void *>(base + operation_offset));
    }
    ::std::ptrdiff_t const stack_size_offset = operation_offset + _roundUp(operation_length);
    if (stack_size != nullptr)
    {
      *stack_size = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + stack_size_offset)));
    }
    ::std::ptrdiff_t const description_length_offset = stack_size_offset + sizeof(::std::uint32_t);
    ::std::uint32_t description_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + operation_length_offset)));
    if (description != nullptr && description_length > 0)
    {
      ::std::ptrdiff_t const description_offset = description_length_offset + sizeof(::std::uint32_t);
      *description = static_cast<char *>(static_cast<void *>(base + description_offset));
    }

    _poped += context_task_length;

    return true;
  }
  else
  {
    void * address = nullptr;
    ::std::size_t length = _reader.consume(&address);
    if (length == 0)
    {
      return false;
    }

    _poped = static_cast<unsigned char *>(address);
    _consumed = static_cast<unsigned char *>(address) + length;

    unsigned char * base = _poped;

    ::std::uint32_t context_task_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base)));

    ::std::ptrdiff_t const operation_length_offset = sizeof(::std::uint32_t);
    ::std::uint32_t operation_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + operation_length_offset)));
    ::std::ptrdiff_t const operation_offset = operation_length_offset + sizeof(::std::uint32_t);
    if (operation != nullptr)
    {
      *operation = static_cast<Operation *>(static_cast<void *>(base + operation_offset));
    }
    ::std::ptrdiff_t const stack_size_offset = operation_offset + _roundUp(operation_length);
    if (stack_size != nullptr)
    {
      *stack_size = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + stack_size_offset)));
    }
    ::std::ptrdiff_t const description_length_offset = stack_size_offset + sizeof(::std::uint32_t);
    ::std::uint32_t description_length = *(static_cast<::std::uint32_t *>(static_cast<void *>(base + operation_length_offset)));
    if (description != nullptr && description_length > 0)
    {
      ::std::ptrdiff_t const description_offset = description_length_offset + sizeof(::std::uint32_t);
      *description = static_cast<char *>(static_cast<void *>(base + description_offset));
    }

    _poped += context_task_length;

    return true;
  }
}

ContextTaskQueue::TaskDeallocator::TaskDeallocator(ContextTaskQueue & context_task_queue)
  : _buffer_reader(&(context_task_queue._reader))
{}

}
}
