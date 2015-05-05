#ifndef NET_SSL_H_
#define NET_SSL_H_

#include <string>
#include <functional>
#include <memory>

#include "boost/asio.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/asio/ssl.hpp"

namespace net {

class ssl : public std::enable_shared_from_this<ssl> {
public:
  typedef std::shared_ptr<ssl> ssl_ptr;
  typedef std::function<void(ssl_ptr, boost::system::error_code)> connect_cb;

  ssl(boost::asio::io_service& io_service,
      std::string host, std::string port,
      boost::posix_time::time_duration timeout);

  ~ssl();

  void connect(connect_cb cb);

  void cancel();

  void connect(ssl_ptr self, connect_cb cb);

  void resolve(ssl_ptr self, connect_cb cb);

  void on_resolve(
      ssl_ptr self,
      connect_cb cb,
      boost::system::error_code ec,
      boost::asio::ip::tcp::resolver::iterator iterator);

  void on_connect(
      ssl_ptr self,
      connect_cb cb,
      boost::system::error_code ec,
      boost::asio::ip::tcp::resolver::iterator);

  void on_handshake(ssl_ptr self, connect_cb cb, boost::system::error_code ec);

  void timer_callback(ssl_ptr, boost::system::error_code const& ec);

  void finally(boost::system::error_code const& ec);

  boost::asio::ssl::context ctx_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  boost::asio::deadline_timer req_timeout_timer_;
  std::string host_, port_;
  bool connected_;
};

}  // namespace net

#endif  // NET_SSL_H_