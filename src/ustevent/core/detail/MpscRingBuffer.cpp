#include "ustevent/core/detail/MpscRingBuffer.h"

#include <cassert>
#include "ustevent/core/detail/Macros.h"
#include "ustevent/core/detail/Instructions.h"

namespace ustevent
{
namespace detail
{

MpscRingBuffer::MpscRingBuffer(::std::size_t capacity, unsigned int producer_num)
  : _capacity(capacity)
  , _producer_num(producer_num)
{
  _workers.resize(producer_num);
  _buffer.resize(_capacity);
}

MpscRingBuffer::Worker::Worker() = default;

MpscRingBuffer::Worker::Worker(Worker const&)
{
  // dummy copy constructor only called by resize
}

auto MpscRingBuffer::current() const
  -> RingBufferOffset
{
  return _consumer_barrier.load(::std::memory_order_relaxed);
}

auto MpscRingBuffer::registerWorker(unsigned int i)
  -> MpscRingBuffer::Worker *
{
  auto & worker = _workers[i];
  worker._seen.store(MAX_OFFSET, ::std::memory_order_relaxed);
  worker._registered.store(true, ::std::memory_order_release);
  return &worker;
}

void MpscRingBuffer::unregisterWorker(Worker & worker)
{
  worker._registered.store(false, ::std::memory_order_relaxed);
}

auto MpscRingBuffer::registered(MpscRingBuffer::Worker const& worker) const
  -> bool
{
  return worker._registered.load(::std::memory_order_relaxed);
}

auto MpscRingBuffer::acquire(MpscRingBuffer::Worker & worker, ::std::size_t size, void * * address)
  -> bool
{
  if (size > _capacity)
  {
    return false;
  }
  assert(worker._seen.load(::std::memory_order_relaxed) == MAX_OFFSET);

  RingBufferOffset seen;
  RingBufferOffset next;
  RingBufferOffset target;
  RingBufferOffset written;
  do
  {
    // Get the global stable next offset. Save the it in Worker,
    // but lock WRAP_LOCK_BIT to mark the value as unstable.
    seen = _getStableNext();
    next = _getBufferIndex(seen);
    assert(next < _capacity);
    worker._seen.store(_lockWrapBit(next), ::std::memory_order_relaxed);

    // Compute the target offset.
    target = next + size;
    written = current();
    // We cannot go beyond the _consumer_barrier offset or catch up with it.
    if (USTEVENT_UNLIKELY(next < written && target >= written))
    {
      worker._seen.store(MAX_OFFSET, ::std::memory_order_release);
      return false;
    }

    if (USTEVENT_UNLIKELY(target >= _capacity))
    {
      // If we would exceed the buffer, then attempt to lock the WRAP_LOCK_BIT
      // and use the space in the beginning. If we used all space exactly to the end,
      // then reset to 0.
      target = target > _capacity ? _lockWrapBit(size) : 0;
      if (_getBufferIndex(target) >= written)
      {
        worker._seen.store(MAX_OFFSET, ::std::memory_order_release);
        return false;
      }
      // Increment the wrap-around counter.
      _setWrapCounter(target, _increaseWrapCounter(_getWrapCounter(seen)));
    }
    else
    {
      // Preserve the wrap-around counter.
      _setWrapCounter(target, _getWrapCounter(seen));
    }
  } while (!_producer_barrier.compare_exchange_weak(seen, target));

  // Acquire the range. The _seen is stable now.
  worker._seen.store(_unlockWrapBit(worker._seen.load(::std::memory_order_relaxed)), ::std::memory_order_relaxed);

  // If target is unstable (because of wrapping), then save the _end and mark stable
  if (USTEVENT_UNLIKELY(_isLocked(target)))
  {
    // cannot wrap-around again if Consumer dit not catch-up.
    assert(current() <= next);
    assert(_end == MAX_OFFSET);
    _end = next;
    next = 0;

    // Use release memory order to ensure the _end reaches global visibility before mark stable.
    _producer_barrier.store(_unlockWrapBit(target), ::std::memory_order_release);
  }

  assert(_getBufferIndex(target) <= _capacity);
  if (address != nullptr)
  {
    *address = &_buffer[next];
  }
  return true;
}

void MpscRingBuffer::produce(MpscRingBuffer::Worker & worker)
{
  assert(worker._registered.load(::std::memory_order_relaxed));
  assert(worker._seen.load(::std::memory_order_relaxed) != MAX_OFFSET);
  worker._seen.store(MAX_OFFSET, ::std::memory_order_release);
}

auto MpscRingBuffer::consume(void * * address)
  -> ::std::size_t
{
  RingBufferOffset next;
  RingBufferOffset ready;
  RingBufferOffset written = current();
  do
  {
    next = _getBufferIndex(_getStableNext());
    if (written == next)
    {
      // if producers did not advance. then nothing to do.
      return 0;
    }

    // Observe the ready offset of each worker.
    ready = MAX_OFFSET;
    for (auto & worker : _workers)
    {
      // Skip if the worker has not registered.
      if (!registered(worker))
      {
        continue;
      }
      // Get a stable seen value. This is nessary since we want to discard the stale seen value.
      RingBufferOffset seen = _getStableSeen(worker);
      // We are interested in the smallest senn offset that is not behind the written offset.
      // Ignore the offsets after the possible wrap-around.
      if (seen >= written)
      {
        ready = ::std::min(seen, ready);
      }
      assert(ready >= written);
    }

    // We need to determine whether wrap-around occurred and deduct the safe ready offset
    if (next < written)
    {
      // Wrap-around case.
      // Check for the cut off first.
      RingBufferOffset end = ::std::min(_capacity, _end);
      if (ready == MAX_OFFSET && written == end)
      {
        if (_end != MAX_OFFSET)
        {
          _end = MAX_OFFSET;
        }

        // Wrap-around the consumer and start from zero.
        written = 0;
        _consumer_barrier.store(written, ::std::memory_order_release);
        continue;
      }

      assert(ready > next);
      ready = ::std::min(ready, end);
      assert(ready >= written);
    }
    else
    {
      // Regular case.
      // Up to the observed ready (if set) or the next offset
      ready = ::std::min(ready, next);
    }
    ::std::size_t size = ready - written;
    if (address != nullptr)
    {
      *address = &_buffer[written];
    }
    assert(ready >= written);
    assert(size <= _capacity);
    return size;
  } while (true);
}

void MpscRingBuffer::release(::std::size_t bytes)
{
  RingBufferOffset written = current();
  RingBufferOffset released_written = written + bytes;

  assert(written <= _capacity);
  assert(written <= _end);
  assert(released_written <= _capacity);

  _consumer_barrier.store((released_written == _capacity) ? 0 : released_written);
}

auto MpscRingBuffer::_getStableSeen(MpscRingBuffer::Worker const& worker)
  -> RingBufferOffset
{
  RingBufferOffset seen;
  do
  {
    seen = worker._seen.load(::std::memory_order_acquire);
    if (_isLocked(seen))
    {
      pause();
      continue;
    }
    return seen;
  } while (true);
}

auto MpscRingBuffer::_getStableNext() const
  -> RingBufferOffset
{
  RingBufferOffset next;
  do
  {
    next = _producer_barrier.load(::std::memory_order_acquire);
    if (_isLocked(next))
    {
      pause();
      continue;
    }
    assert(_getBufferIndex(next) < _capacity);
    return next;
  } while (true);
}

MpscRingBufferWriter::MpscRingBufferWriter()
  : _buffer(nullptr)
  , _worker(nullptr)
{}

MpscRingBufferWriter::MpscRingBufferWriter(MpscRingBuffer & ring_buffer, unsigned int i)
  : _buffer(&ring_buffer)
  , _worker(ring_buffer.registerWorker(i))
{}

MpscRingBufferWriter::~MpscRingBufferWriter() noexcept
{
  if (_buffer && _worker)
  {
    _buffer->unregisterWorker(*_worker);
  }
  _buffer = nullptr;
  _worker = nullptr;
}

MpscRingBufferWriter::MpscRingBufferWriter(MpscRingBufferWriter && rhs)
{
  _buffer = rhs._buffer;
  _worker = rhs._worker;
  rhs._buffer = nullptr;
  rhs._worker = nullptr;
}

auto MpscRingBufferWriter::operator=(MpscRingBufferWriter && rhs)
  -> MpscRingBufferWriter &
{
  if (this != &rhs)
  {
    _buffer = rhs._buffer;
    _worker = rhs._worker;
    rhs._buffer = nullptr;
    rhs._worker = nullptr;
  }
  return *this;
}

auto MpscRingBufferWriter::acquire(::std::size_t size, void * * address)
  -> bool
{
  if (_buffer && _worker)
  {
    return _buffer->acquire(*_worker, size, address);
  }
  return false;
}

void MpscRingBufferWriter::produce()
{
  if (_buffer && _worker)
  {
    _buffer->produce(*_worker);
  }
}

MpscRingBufferReader::MpscRingBufferReader(MpscRingBuffer & ring_buffer)
  : _buffer(&ring_buffer)
{}

auto MpscRingBufferReader::consume(void * * address)
  -> ::std::size_t
{
  assert(_buffer);
  return _buffer->consume(address);
}

void MpscRingBufferReader::release(::std::size_t bytes)
{
  assert(_buffer);
  _buffer->release(bytes);
}

}
}
