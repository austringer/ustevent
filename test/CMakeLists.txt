add_executable(UsteventUnitTest
  TestMain.cpp
  TestConcurrency.cpp
  TestContext.cpp
  TestTcp.cpp
)

target_link_libraries(UsteventUnitTest
  PRIVATE ustevent_core
  PRIVATE ustevent_net
)

set_target_properties(UsteventUnitTest
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test"
)

add_test(
  NAME UsteventUnitTest
  COMMAND $<TARGET_FILE:UsteventUnitTest>
)
