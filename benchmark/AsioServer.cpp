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

class Session : public ::std::enable_shared_from_this<Session>
{
public:
  Session(::boost::asio::io_service & ios, ::std::size_t total_count)
    : _io_service(ios)
    , _socket(ios)
    , _total_count(total_count)
  {}

  Session(Session const&) = delete;
  auto operator=(Session const&) -> Session & = delete;

  auto socket()
    -> ::boost::asio::ip::tcp::socket &
  {
    return _socket;
  }

  void start()
  {
    ::boost::asio::ip::tcp::no_delay no_delay(true);
    _socket.set_option(no_delay);
    readHead();
  }

  void write()
  {
    Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
    ::std::uint32_t full_size = header->getFullSize();
    header->increasePacketSequence();

    ::boost::asio::async_write(_socket, ::boost::asio::buffer(_buffer, full_size),
      [this, self = shared_from_this()](::boost::system::error_code const& err, ::std::size_t /*cb*/)
      {
        if (!err)
        {
          if (checkCount())
          {
            _socket.close();
          }
          else
          {
            readHead();
          }
        }
      });
  }

  void readHead() {
    ::boost::asio::async_read(_socket, ::boost::asio::buffer(_buffer, sizeof(Header)),
      [this, self = shared_from_this()](::boost::system::error_code const& err, ::std::size_t /*cb*/)
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
    ::std::uint32_t body_size = header->getBodySize();
    ::boost::asio::async_read(_socket, ::boost::asio::buffer(_buffer + sizeof(Header), body_size),
      [this, self = shared_from_this()](::boost::system::error_code const& err, ::std::size_t /*cb*/)
      {
        if (!err)
        {
          if (checkCount())
          {
            _socket.close();
          }
          else
          {
            write();
          }
        }
      });
  }

  auto checkCount()
    -> bool
  {
    Header * header = static_cast<Header *>(static_cast<void *>(_buffer));
    return (header->getPacketSequence() >= _total_count);
  }

private:

  ::boost::asio::io_service &       _io_service;
  ::boost::asio::ip::tcp::socket    _socket;
  ::std::uint32_t const             _total_count;

  static ::std::size_t const        MAX_BUFFER_SIZE = 10000 + sizeof(Header);
  unsigned char                     _buffer[MAX_BUFFER_SIZE];
};

class IoServicePool
{
public:
  explicit
  IoServicePool(::std::size_t pool_size)
    : _next_io_service(0)
  {
    if (pool_size == 0)
    {
      throw std::runtime_error("io_service_pool size is 0");
    }

    for (::std::size_t i = 0; i < pool_size; ++i)
    {
      auto io_service = ::std::make_shared<::boost::asio::io_service>();
      auto work = ::std::make_shared<::boost::asio::io_service::work>(*io_service);
      _io_services.push_back(io_service);
      _works.push_back(work);
    }
  }

  void run()
  {
    ::std::vector<::std::shared_ptr<::std::thread>> threads;
    for (::std::size_t i = 0; i < _io_services.size(); ++i)
    {
      auto thread = ::std::make_shared<::std::thread>(
        [this, i]()
        {
          _io_services[i]->run();
        });
      threads.push_back(thread);
    }

    for (::std::size_t i = 0; i < _io_services.size(); ++i)
    {
      threads[i]->join();
    }
  }

  void stop()
  {
    for (std::size_t i = 0; i < _works.size(); ++i)
    {
      _works[i].reset();
      _io_services[i]->stop();
      _io_services[i]->reset();
    }
  }

  auto getIoService()
    -> ::boost::asio::io_service &
  {
    auto & io_service = *_io_services[_next_io_service];
    ++_next_io_service;
    if (_next_io_service == _io_services.size())
    {
      _next_io_service = 0;
    }
    return io_service;
  }

  auto getIoService(::std::size_t index)
    -> ::boost::asio::io_service &
  {
    index = index % _io_services.size();
    return *_io_services[index];
  }

  IoServicePool(IoServicePool const&) = delete;
  auto operator=(IoServicePool const&) = delete;

private:
  using IoServicePtr  = ::std::shared_ptr<::boost::asio::io_service>;
  using WorkPtr       = ::std::shared_ptr<::boost::asio::io_service::work>;

  ::std::vector<IoServicePtr>   _io_services;
  ::std::vector<WorkPtr>        _works;
  ::std::size_t                 _next_io_service;
};

class Server
{
public:
  Server(::std::string_view host, unsigned short port, ::std::size_t thread_count, ::std::uint32_t total_count)
    : _thread_count(thread_count)
    , _total_count(total_count)
    , _io_service_pool(thread_count)
    , _acceptor(_io_service_pool.getIoService())
  {
    auto endpoint = ::boost::asio::ip::tcp::endpoint(::boost::asio::ip::address::from_string(host.data()), port);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(::boost::asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();
  }

  void start()
  {
    accept();
  }

  void wait()
  {
    _io_service_pool.run();
  }

  Server(Server const&) = delete;
  auto operator=(Server const&) -> Server & = delete;

private:
  void accept()
  {
    auto new_session = ::std::make_shared<Session>(_io_service_pool.getIoService(), _total_count);
    auto & socket = new_session->socket();
    _acceptor.async_accept(socket,
      [this, new_session=::std::move(new_session)](::boost::system::error_code const& err)
      {
        if (!err)
        {
          new_session->start();
          accept();
        }
      });
  }

  ::std::size_t const               _thread_count;
  ::std::uint32_t const             _total_count;

  IoServicePool                     _io_service_pool;

  ::boost::asio::ip::tcp::acceptor  _acceptor;
};

}

void startServer(::std::string_view host, unsigned short port, ::std::size_t thread_count, ::std::uint32_t total_count)
{
  Server s(host, port, thread_count, total_count);
  s.start();
  s.wait();
}

int main(int argc, char * argv[])
{
  if (argc != 5)
  {
    ::std::cerr << "Usage: AsioServer <host> <port> <threads> <totalcount>" << ::std::endl;
    return 1;
  }

  char const* host = argv[1];
  unsigned short port = ::std::atoi(argv[2]);
  ::std::size_t thread_count = ::std::atoi(argv[3]);
  unsigned int total_count = ::std::atoi(argv[4]);

  startServer(host, port, thread_count, total_count);
}
