#pragma once

#include <functional>
#include <memory>
#include <string>

#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

#include "net/tcp.h"

namespace motis {
namespace client {

class motis_con : public net::tcp, boost::asio::coroutine {
public:
  typedef std::function<void(net::tcp::tcp_ptr, std::string,
                             boost::system::error_code)>
      callback;

  motis_con(boost::asio::io_service& ios, std::string host, std::string port,
            boost::posix_time::time_duration timeout);

  void query(std::string const& req, callback cb);

protected:
  void transfer(net::tcp::tcp_ptr self, callback cb,
                boost::system::error_code ec);

  void respond(callback cb, net::tcp::tcp_ptr self,
               boost::system::error_code ec);

  std::size_t copy_content(std::size_t buffer_size);

  int32_t request_size_;
  std::string request_;

  int32_t response_size_;
  boost::asio::streambuf buf_;
  std::vector<char> response_;
};

}  // namespace client
}  // namespace motis
