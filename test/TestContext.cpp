#include "catch2/catch.hpp"

#include <iostream>
#include <chrono>
#include <thread>

#include "ustevent/core/fiber/Sleep.h"
#include "ustevent/core/thread/Future.h"
#include "ustevent/core/thread/Sleep.h"
#include "ustevent/core/Context.h"
#include "ustevent/core/BlockingContextStrategy.h"

using namespace ustevent;

SCENARIO("Test Ustevent Context", "[core]")
{
  THEN("post a lambda into Context to change its value")
  {
    Context ctx(1, Context::DEBUG_ON);

    ::std::thread t([&ctx]() { ctx.run<BlockingContextStrategy>(ctx); });

    int i = 0;
    fiber::Promise<void> finished;

    ctx.run();

    ctx.post([&i](){
      i += 100;
      // fiber::sleepFor(::std::chrono::seconds(1000));
    });

    ctx.post([&i, &finished](){
      i += 100;
      finished.set_value();
    });
    finished.get_future().wait();

    REQUIRE(i == 200);

    ctx.terminate();
    t.join();
  }

  THEN("call a lambda with Context to change its value")
  {
    Context ctx(1, Context::DEBUG_ON);
    ::std::thread t([&ctx]() { ctx.run<BlockingContextStrategy>(ctx); });

    int i = 0;
    thread::Promise<void> finished;

    ctx.run();

    auto ret = ctx.call([&i](){
      i = 100;
      return i;
    });
    REQUIRE(i == 100);
    REQUIRE(ret == 100);

    ctx.terminate();
    t.join();
  }
}
