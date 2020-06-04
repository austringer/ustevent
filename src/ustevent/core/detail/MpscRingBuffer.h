#ifndef USTEVENT_CORE_DETAIL_MPSCRINGBUFFER_H_
#define USTEVENT_CORE_DETAIL_MPSCRINGBUFFER_H_

#include <cstdint>
#include <atomic>
#include <limits>
#include <vector>

namespace ustevent
{
namespace detail
{

class MpscRingBuffer
{
public:

  //  1 bit   LOCK_BIT
  // 31 bit   WRAP COUNTER
  // 32 bit   BUFFER INDEX
  using RingBufferOffset = ::std::uint64_t;

  struct Worker
  {
    ::std::atomic<RingBufferOffset>   _seen         = { 0 };
    ::std::atomic<bool>               _registered   = { false };

    Worker();
    Worker(Worker const&);
  };

  MpscRingBuffer(::std::size_t capacity, unsigned int producer_num);

  auto registerWorker(unsigned int i)
    -> Worker *;

  void unregisterWorker(Worker & worker);

  auto registered(Worker const& worker) const
    -> bool;

  auto current() const
    -> RingBufferOffset;

  auto acquire(Worker & worker, ::std::size_t size, void * * address)
    -> bool;

  void produce(Worker & worker);

  auto consume(void * * address)
    -> ::std::size_t;

  void release(::std::size_t bytes);

private:

  enum {
    INDEX_MASK	        = 0x00000000ffffffffUL,
    WRAP_COUNTER_MASK   = 0x7fffffff00000000UL,
    WRAP_LOCK_BIT	      = 0x8000000000000000UL,
    MAX_OFFSET          = ::std::numeric_limits<RingBufferOffset>::max() & ~WRAP_LOCK_BIT,
  };

  ::std::size_t                             _capacity;
  unsigned int                              _producer_num;

  volatile
  ::std::atomic<RingBufferOffset>           _producer_barrier     = { 0 };

  RingBufferOffset                          _end                  = MAX_OFFSET;

  ::std::atomic<RingBufferOffset>           _consumer_barrier     = { 0 };

  ::std::vector<Worker>                     _workers;

  ::std::vector<unsigned char>              _buffer;

  static
  auto _getStableSeen(Worker const& worker)
    -> RingBufferOffset;

  auto _getStableNext() const
    -> RingBufferOffset;

  static inline
  auto _unlockWrapBit(RingBufferOffset offset)
    -> RingBufferOffset
  {
    return offset & ~WRAP_LOCK_BIT;
  }

  static inline
  auto _lockWrapBit(RingBufferOffset offset)
    -> RingBufferOffset
  {
    return offset | WRAP_LOCK_BIT;
  }

  static inline
  auto _isLocked(RingBufferOffset offset)
    -> bool
  {
    return (offset & WRAP_LOCK_BIT) != 0;
  }

  static inline
  auto _getWrapCounter(RingBufferOffset offset)
    -> RingBufferOffset
  {
    return offset & WRAP_COUNTER_MASK;
  }

  static inline
  void _setWrapCounter(RingBufferOffset offset, RingBufferOffset wrap_counter)
  {
    offset |= wrap_counter;
  }

  static inline
  auto _increaseWrapCounter(RingBufferOffset wrap_counter)
    -> RingBufferOffset
  {
    return _getWrapCounter(wrap_counter + 0x100000000UL);
  }

  static inline
  auto _getBufferIndex(RingBufferOffset offset)
    -> RingBufferOffset
  {
    return offset & INDEX_MASK;
  }

};

class MpscRingBufferWriter
{
public:
  MpscRingBufferWriter();

  MpscRingBufferWriter(MpscRingBuffer & ring_buffer, unsigned int i);

  ~MpscRingBufferWriter() noexcept;

  MpscRingBufferWriter(MpscRingBufferWriter && rhs);
  auto operator=(MpscRingBufferWriter && rhs)
    -> MpscRingBufferWriter &;

  auto acquire(::std::size_t size, void * * address)
    -> bool;

  void produce();

private:
  MpscRingBuffer          * _buffer;
  MpscRingBuffer::Worker  * _worker;
};

class MpscRingBufferReader
{
public:
  explicit
  MpscRingBufferReader(MpscRingBuffer & ring_buffer);

  auto consume(void * * address)
    -> ::std::size_t;

  void release(::std::size_t bytes);

private:
  MpscRingBuffer          * _buffer;
};

}
}

#endif
