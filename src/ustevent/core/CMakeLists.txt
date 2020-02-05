set(ustevent_core_Sources
  Context.cpp
  Fiber.cpp
  FiberDebugInfo.cpp
  ContextStrategy.cpp
  BlockingContextStrategy.cpp
)

add_library(ustevent_core STATIC ${ustevent_core_Sources})
set_property(TARGET ustevent_core PROPERTY POSITION_INDEPENDENT_CODE ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(ustevent_core
  PUBLIC  Threads::Threads
  PRIVATE boost_context
  PRIVATE boost_fiber
  PRIVATE unwind
)
