#include <chrono>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#if defined(__linux__)
#include <arpa/inet.h>
#endif

namespace {

#pragma pack(push, 1)
struct Header
{
  ::std::uint32_t _body_size;
  ::std::uint32_t _packet_sequence;

  auto getBodySize() const
    -> ::std::uint32_t
  {
    return ::ntohl(_body_size);
  }

  auto getFullSize() const
    -> ::std::uint32_t
  {
    return sizeof(Header) + ::ntohl(_body_size);
  }

  auto getPacketSequence() const
    -> ::std::uint32_t
  {
    return ::ntohl(_packet_sequence);
  }

  void increasePacketSequence()
  {
    ::std::uint32_t sequence = getPacketSequence() + 1;
    _packet_sequence = ::htonl(sequence);
  }
};
#pragma pack(pop)

auto clockUs()
  -> ::std::uint64_t
{
  return ::std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
}

class Session
{
public:
  Session(::boost::asio::io_service & ios, std::uint32_t body_size, ::std::size_t total_count)
    : _io_service(ios)
    , _socket(ios)
    , _body_size(body_size)
    , _total_count(total_count)
  {}

  Session(Session const&) = delete;
  auto operator=(Session const&) -> Session & = delete;

  void write()
  {
    Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
    ::std::uint32_t full_size = header->getFullSize();
    ::boost::asio::async_write(_socket, ::boost::asio::buffer(_buffer, full_size),
      [this](::boost::system::error_code const& err, ::std::size_t /*cb*/)
      {
        if (!err)
        {
          readHead();
        }
      });
  }

  void readHead()
  {
    ::boost::asio::async_read(_socket, ::boost::asio::buffer(_buffer, sizeof(Header)),
      [this](::boost::system::error_code const& err, ::std::size_t /*cb*/)
      {
        if (!err)
        {
          readBody();
        }
      });
  }

  void readBody()
  {
    Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
    size_t body_size = header->getBodySize();
    ::boost::asio::async_read(_socket, ::boost::asio::buffer(_buffer + sizeof(Header), body_size),
      [this](::boost::system::error_code const& err, ::std::size_t /*cb*/)
      {
        if (!err)
        {
          if (_checkCount())
          {
            _finished = true;
            _stop_time = clockUs();
            _socket.close();
          }
          else
          {
            Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
            header->_body_size = ::htonl(_body_size);
            header->increasePacketSequence();

            write();
          }
        }
      });
  }

  void start(::boost::asio::ip::tcp::endpoint const& endpoint)
  {
    _socket.async_connect(endpoint, [this](::boost::system::error_code const& err)
    {
      if (!err)
      {
        _socket.set_option(::boost::asio::ip::tcp::no_delay(true));

        Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
        header->_body_size = ::htonl(_body_size);
        header->_packet_sequence = ::htonl(1);

        _start_time = clockUs();
        write();
      }
    });
  }

  auto finished() const
    -> bool
  {
    return _finished;
  }

  auto sumUsedTime() const
    -> ::std::uint64_t
  {
    return _stop_time - _start_time;
  }
private:

  auto _checkCount()
    -> bool
  {
    Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
    return (header->getPacketSequence() >= _total_count);
  }
private:
  ::boost::asio::io_service &       _io_service;
  ::boost::asio::ip::tcp::socket    _socket;
  ::std::uint32_t const             _body_size;
  ::std::uint32_t const             _total_count;

  static ::std::size_t const MAX_BUFFER_SIZE    = 10000 + sizeof(Header);
  unsigned char                     _buffer[MAX_BUFFER_SIZE];

  bool                              _finished     = false;
  ::std::uint64_t                   _start_time   = 0;
  ::std::uint64_t                   _stop_time    = 0;
};

class Client
{
public:
  Client(::std::string_view host, unsigned short port,
    ::std::size_t thread_count, ::std::size_t session_count,
    ::std::uint32_t body_size, ::std::uint32_t total_count)
    : _thread_count(thread_count)
  {
    _io_services.reserve(thread_count);
    _threads.reserve(thread_count);
    _sessions.reserve(session_count);

    for (unsigned int i = 0; i < _thread_count; ++i)
    {
      _io_services.push_back(::std::make_shared<::boost::asio::io_service>());
    }

    _endpoint = ::boost::asio::ip::tcp::endpoint(::boost::asio::ip::address::from_string(host.data()), port);

    for (::std::size_t i = 0; i < session_count; ++i)
    {
      auto & io_service = *_io_services[i % _thread_count];
      _sessions.push_back(::std::make_shared<Session>(io_service, body_size, total_count));
    }
  }

  ~Client() noexcept
  {
    ::std::uint32_t finished_count = 0;
    ::std::uint32_t error_count = 0;
    ::std::uint64_t total_time = 0;
    for (auto & session : _sessions)
    {
      if (session->finished())
      {
        ++finished_count;
        total_time += session->sumUsedTime();
      }
      else
      {
        ++error_count;
      }
    }
    ::std::cout << "failed count: " << error_count << "\n";
    ::std::cout << "finished count: " << finished_count << "\n";
    ::std::cout << "average time(us): "
                << static_cast<::std::uint64_t>(static_cast<double>(total_time) / finished_count) << ::std::endl;
  }

  Client(Client const&) = delete;
  auto operator=(Client const&) -> Client & = delete;

  void start()
  {
    for (auto & session : _sessions)
    {
      session->start(_endpoint);
    }
    for (unsigned int i = 0; i < _thread_count; ++i)
    {
      _threads.push_back(::std::make_shared<::std::thread>(
        [this, i]()
        {
          auto & io_service = *_io_services[i];
          io_service.run();
        }));
    }
  }

  void wait()
  {
    for (auto & thread : _threads)
    {
      thread->join();
    }
  }

private:
  ::std::size_t const                                       _thread_count;
  ::std::vector<std::shared_ptr<::boost::asio::io_service>> _io_services;
  ::std::vector<std::shared_ptr<::std::thread>>             _threads;
  ::std::vector<std::shared_ptr<Session>>                   _sessions;
  ::boost::asio::ip::tcp::endpoint                          _endpoint;
};

}

void startClient(
  ::std::string_view host, unsigned short port,
  ::std::size_t thread_count, ::std::size_t session_count,
  ::std::uint32_t body_size, ::std::uint32_t total_count)
{
  Client c(host, port, thread_count, session_count, body_size, total_count);
  c.start();
  c.wait();
}

int main(int argc, char * argv[])
{
  if (argc != 7)
  {
    ::std::cerr << "Usage: AsioClient <host> <port> <threads> <sessions> <bodysize> <totalcount>" << ::std::endl;
    return 1;
  }

  char const* host = argv[1];
  unsigned short port = ::std::atoi(argv[2]);
  ::std::size_t thread_count = ::std::atoi(argv[3]);
  ::std::size_t session_count = ::std::atoi(argv[4]);
  unsigned int body_size = ::std::atoi(argv[5]);
  unsigned int total_count = ::std::atoi(argv[6]);

  startClient(host, port, thread_count, session_count, body_size, total_count);
}
