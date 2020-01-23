add_executable(UsteventUnitTest
  CatchMain.cpp
  TestContext.cpp
)

target_link_libraries(UsteventUnitTest
  PRIVATE ustevent_core
)

set_target_properties(UsteventUnitTest
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test"
)

add_test(
  NAME UsteventUnitTest
  COMMAND $<TARGET_FILE:UsteventUnitTest>
)
