cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(ustevent LANGUAGES CXX)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_COMPILER_LAUNCHER ccache)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-pedantic -rdynamic -Wall -Wextra -Werror \
  -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


set(CMAKE_VERBOSE_MAKEFILE ON)
message("Generator is set to ${CMAKE_GENERATOR}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ustevent_SOURCE_DIR}/out/lib")   # .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ustevent_SOURCE_DIR}/out/bin")   # executable .exe .dll

if(!WIN32)
  set(CMAKE_EXECUTABLE_SUFFIX ".bin")
endif()

include_directories(
  "${ustevent_SOURCE_DIR}/src"
  "${ustevent_SOURCE_DIR}/example"
  "${ustevent_SOURCE_DIR}/third_party/src"
  "${ustevent_BINARY_DIR}/src"
  "${ustevent_BINARY_DIR}/example"
)

link_directories(
  "${ustevent_SOURCE_DIR}/lib"
)

# include("${ustevent_SOURCE_DIR}/.cmake/ToolChainSetting.cmake")
# include("${ustevent_SOURCE_DIR}/.cmake/GenerateService.cmake")

add_subdirectory(src)
add_subdirectory(test)
# add_subdirectory(example)
