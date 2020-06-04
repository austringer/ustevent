#ifndef USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_H_
#define USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_H_

#include <atomic>
#include "ustevent/core/detail/ContextOperation.h"
#include "ustevent/core/detail/MpscRingBuffer.h"
#include "ustevent/core/concurrency/detail/ObjectScopedThreadLocalStorage.h"

namespace ustevent
{
namespace detail
{

class ContextTaskQueue
{
private:
  /*
   * |      4 bytes task length       |   4 bytes operation length    |
   * |                            operation                           |
   * |                                        |  padding to 8 bytes   |
   * |      4 bytes stack size        |   4 bytes description length  |
   * |                           description                          |
   * |                                        |  padding to 8 bytes   |
   */

public:

  class TaskDeallocator
  {
  public:
    explicit
    TaskDeallocator(ContextTaskQueue & context_task_queue);

    template <typename Task>
    void deallocate(Task * address);

  private:
    MpscRingBufferReader * _buffer_reader;
  };

  explicit
  ContextTaskQueue(::std::size_t buffer_length);

  template <typename Op>
  void push(Op && operation, ::std::size_t stack_size, ::std::string_view description);

  auto pop(Operation * * operation, ::std::size_t * stack_size, ::std::string_view * description)
    -> bool;

private:

  MpscRingBuffer                  _ring_buffer;

  ::std::atomic<unsigned int>     _writer_sequence = { 0 };

  using OstlsMpscRingBufferWriter = ObjectScopedThreadLocalStorage<MpscRingBufferWriter, ContextTaskQueue>;
  OstlsMpscRingBufferWriter       _writers;

  MpscRingBufferReader            _reader;

  unsigned char *                 _consumed = nullptr;
  unsigned char *                 _poped    = nullptr;

  static inline constexpr
  auto _roundUp(::std::size_t num)
    -> ::std::size_t;
};

}
}

#include "ustevent/core/detail/ContextTaskQueue.hpp"

#endif // USTEVENT_CORE_DETAIL_CONTEXTTASKQUEUE_H_
