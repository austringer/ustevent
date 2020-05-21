#include "doctest/doctest.h"

#include <atomic>
#include <list>
#include <set>
#include <iostream>
#include <chrono>
#include <thread>

#include "ustevent/core/concurrency/SpscListQueue.h"
#include "ustevent/core/concurrency/SpmcListQueue.h"
#include "ustevent/core/thread/Barrier.h"

using namespace ustevent;

static int ENQUEUE_DATA_NUM = 1000000;

SCENARIO("Test Ustevent concurrency library")
{
  WHEN("Test SpscListQueue")
  {
    thread::Barrier barrier(2);
    SpscListQueue<::std::string> queue;
    auto p1 = ::std::thread([&barrier, &queue]() {
      barrier.wait();
      for (int i = 0; i < ENQUEUE_DATA_NUM; ++i)
      {
        queue.enqueue(::std::to_string(i));
      }
    });

    auto c1 = ::std::thread([&barrier, &queue]() {
      ::std::string data;
      int i = 0;
      ::std::set<int> datas;
      for (int i = 0; i < ENQUEUE_DATA_NUM; ++i)
      {
        datas.insert(i);
      }
      barrier.wait();
      while (true)
      {
        if (queue.dequeue(data))
        {
          datas.erase(::std::stoi(data));
          ++i;
        }
        if (i == ENQUEUE_DATA_NUM)
        {
          REQUIRE(datas.empty());
          break;
        }
      }
    });

    p1.join();
    c1.join();
  }

  WHEN("Test SpmcListQueue")
  {
    int consumer_num = 8;

    thread::Barrier barrier1(1 + consumer_num);
    thread::Barrier barrier2(consumer_num);
    SpmcListQueue<::std::string> queue;
    auto p1 = ::std::thread([&barrier1, &queue]() {
      barrier1.wait();
      for (int i = 0; i < ENQUEUE_DATA_NUM; ++i)
      {
        queue.enqueue(::std::to_string(i));
      }
    });

    ::std::atomic<int> dequeue_num = { 0 };
    ::std::mutex datas_mutex;
    ::std::set<int> datas;
    for (int i = 0; i < ENQUEUE_DATA_NUM; ++i)
    {
      datas.insert(i);
    }
    ::std::list<::std::thread> ct;
    for (int c = 0; c < consumer_num; ++c)
    {
      ct.emplace_back([&barrier1, &queue, &dequeue_num, &datas_mutex, &datas, &barrier2]() {
        ::std::string data;
        ::std::set<int> datas_in_this_thread;
        barrier1.wait();
        while (true)
        {
          if (queue.dequeue(data))
          {
            datas_in_this_thread.insert(::std::stoi(data));
            ++dequeue_num;
          }
          if (dequeue_num.load() == ENQUEUE_DATA_NUM)
          {
            break;
          }
        }
        {
          ::std::scoped_lock<::std::mutex> lock(datas_mutex);
          for (auto & data : datas_in_this_thread)
          {
            datas.erase(data);
          }
        }
        barrier2.wait();
        REQUIRE(datas.empty());
      });
    }

    p1.join();
    for (auto & t : ct)
    {
      t.join();
    }
  }
}
