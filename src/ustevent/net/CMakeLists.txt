set(ustevent_net_Sources
  NetContext.cpp
  MultiplexingContextStrategy.cpp
  detail/Buffer.cpp
  ip/IpAddress.cpp
  tcp/TcpAddress.cpp
  tcp/TcpConnection.cpp
)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(ustevent_net_Sources ${ustevent_net_Sources}
    detail/linux/ContinuousBuffer.cpp
    detail/linux/EpollSelector.cpp
    tcp/detail/linux/TcpDialer.cpp
    tcp/detail/linux/TcpListener.cpp
    tcp/detail/linux/TcpSocket.cpp
    tcp/detail/linux/TcpSyncReadStream.cpp
    tcp/detail/linux/TcpSyncWriteStream.cpp
  )
endif()

add_library(ustevent_net STATIC ${ustevent_net_Sources})
set_property(TARGET ustevent_net PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(ustevent_net
  PUBLIC ustevent_core
)
