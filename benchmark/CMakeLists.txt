set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(AsioClient
  AsioClient.cpp
)

target_compile_definitions(AsioClient PRIVATE
  BOOST_ERROR_CODE_HEADER_ONLY
)

target_link_libraries(AsioClient
  PUBLIC Threads::Threads
)

set_target_properties(AsioClient
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/benchmark"
)

add_executable(AsioServer
  AsioServer.cpp
)

target_compile_definitions(AsioServer PRIVATE
  BOOST_ERROR_CODE_HEADER_ONLY
)

target_link_libraries(AsioServer
  PUBLIC Threads::Threads
)

set_target_properties(AsioServer
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/benchmark"
)
