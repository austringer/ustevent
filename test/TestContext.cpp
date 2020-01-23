#include "catch2/catch.hpp"

#include <thread>

#include "ustevent/core/thread/Future.h"
#include "ustevent/core/Context.h"
#include "ustevent/core/BlockingContextStrategy.h"

using namespace ustevent;

SCENARIO("Test Ustevent Context", "[core]")
{
  GIVEN("a ustevent::Context")
  {
    Context ctx;

    WHEN("ctx is runned in a thread with blocking strategy")
    {
      ::std::thread t([&ctx]() { ctx.run<BlockingContextStrategy>(ctx, true, true); });

      AND_WHEN("given a variable")
      {
        int i = 0;
        thread::Promise<void> finished;

        THEN("post a lambda into ctx to change its value")
        {
          ctx.post([&i, &finished](){ i = 100; finished.set_value(); });
          finished.get_future().wait();

          REQUIRE(i == 100);

          ctx.terminate();
          t.join();
          REQUIRE_FALSE(t.joinable());
        }

        THEN("call a lambda with ctx to change its value")
        {
          auto ret = ctx.call([&i](){ i = 100; return i; });
          REQUIRE(i == 100);
          REQUIRE(ret == 100);

          ctx.terminate();
          t.join();
          REQUIRE_FALSE(t.joinable());
        }
      }
    }
  }
}
