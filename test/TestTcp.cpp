#include "doctest/doctest.h"

#include "cstring"
#if defined(__linux__)
#include <arpa/inet.h>  // htonl, ntohl
#endif

#include "ustevent/core/fiber/Barrier.h"
#include "ustevent/net/NetContext.h"
#include "ustevent/net/MultiplexingContextStrategy.h"
#include "ustevent/net/tcp/TcpAddress.h"
#include "ustevent/net/tcp/TcpListener.h"
#include "ustevent/net/tcp/TcpDialer.h"
#include "ustevent/net/Connection.h"

using namespace ustevent;

SCENARIO("Test Ustevent NetContext")
{
  THEN("test an echo server and an echo client")
  {
    net::NetContext ctx = {};

    ::std::thread t([&ctx]() { ctx.run(); });

    fiber::Barrier b(3);

    ctx.start();

    ctx.post([&ctx, &b]() {
      auto tcp_address = net::TcpAddress::parse("127.0.0.1", 50000);
      REQUIRE(tcp_address != nullptr);

      auto [listener, e0] = net::TcpListener::open(ctx, *tcp_address);
      REQUIRE(listener != nullptr);
      REQUIRE(e0 == 0);

      auto [connection, e1] = listener->accept();
      REQUIRE(connection != nullptr);
      REQUIRE(e1 == 0);

      char buffer[64];
      ::std::size_t next_length = 0;
      while (true)
      {
        auto [received_some_size, e2] = connection->readSome(buffer + next_length, sizeof(buffer) - next_length);
        REQUIRE(e2 == 0);
        ::std::size_t received_message_size = received_some_size - sizeof(::std::uint32_t);

        if (received_some_size < sizeof(::std::uint32_t))
        {
          continue;
        }
        else
        {
          ::std::size_t head = ::ntohl(*reinterpret_cast<::std::uint32_t *>(buffer));

          if (head > received_message_size)
          {
            auto [receive_left, e3] = connection->read(buffer + received_some_size, head - received_message_size);
            REQUIRE(receive_left == head - received_message_size);
            REQUIRE(e3 == 0);
            next_length = 0;
          }
          else if (head < received_message_size)
          {
            next_length = received_message_size - head;
          }
          else
          {
            next_length = 0;
          }

          auto [sent, e4] = connection->write(buffer, sizeof(::std::uint32_t) + head);
          REQUIRE(sent == sizeof(::std::uint32_t) + head);
          REQUIRE(e4 == 0);

          if (next_length > 0)
          {
            ::std::memmove(buffer, buffer + sizeof(::std::uint32_t) + head, next_length);
          }
        }

        break;  // unit test run once
      }
      b.wait();
    });

    ctx.post([&ctx, &b]{
      auto [dialer, e0] = net::TcpDialer::open(ctx);
      REQUIRE(dialer != nullptr);
      REQUIRE(e0 == 0);

      auto tcp_address = net::TcpAddress::parse("127.0.0.1", 50000);

      auto [connection, e1] = dialer->connect(*tcp_address);
      REQUIRE(e1 == 0);

      char const* message = "Hello Ustevent";
      ::std::size_t message_length = ::std::strlen(message);
      ::std::uint32_t send_head = ::htonl(message_length + 1);

      char send_buffer[64];
      ::std::size_t send_buffer_length = 0;
      ::std::memcpy(send_buffer, &send_head, sizeof(send_head));
      send_buffer_length += sizeof(send_head);
      ::std::memcpy(send_buffer + sizeof(send_head), message, message_length);
      send_buffer_length += message_length;
      send_buffer[send_buffer_length] = '\0';
      send_buffer_length += 1;

      auto [head_sent, e2] = connection->write(&send_buffer, send_buffer_length);
      REQUIRE(head_sent == send_buffer_length);
      REQUIRE(e2 == 0);

      ::std::uint32_t receive_head = 0;
      auto [receive_head_size, e3] = connection->read(&receive_head, sizeof(receive_head));
      REQUIRE(receive_head_size == sizeof(receive_head));
      REQUIRE(e3 == 0);

      receive_head = ::ntohl(receive_head);
      REQUIRE(receive_head == message_length + 1);

      char receive_buffer[64];
      auto [receive_message_size, e4] = connection->read(receive_buffer, receive_head);
      REQUIRE(receive_message_size == message_length + 1);
      REQUIRE(e4 == 0);
      REQUIRE(::std::strcmp(send_buffer + sizeof(send_head), receive_buffer) == 0);

      b.wait();
    });

    b.wait();

    ctx.terminate();
    t.join();
  }
}
