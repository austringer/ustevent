set(net_Sources
  NetContext.cpp
  MultiplexingContextStrategy.cpp
  detail/Buffer.cpp
  ip/IPAddress.cpp
  tcp/TCPAddress.cpp
  tcp/TcpConnection.cpp
)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(net_Sources ${net_Sources}
    detail/linux/ContinuousBuffer.cpp
    detail/linux/EpollSelector.cpp
    tcp/detail/linux/TCPSocket.cpp
    tcp/detail/linux/TCPSocketAddress.cpp
    tcp/detail/linux/TcpSyncReadStream.cpp
    tcp/detail/linux/TcpSyncWriteStream.cpp
    tcp/detail/linux/TcpDialer.cpp
    tcp/detail/linux/TcpListener.cpp
  )
endif()

add_library(net STATIC ${net_Sources})
set_property(TARGET net PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(net
  PUBLIC core
)
