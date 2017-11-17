#include "motis/client/motis_con.h"

using boost::system::error_code;
namespace asio = boost::asio;

namespace motis {
namespace client {

motis_con::motis_con(asio::io_service& ios, std::string host, std::string port,
                     boost::posix_time::time_duration timeout)
    : net::tcp(ios, std::move(host), std::move(port), std::move(timeout)),
      request_size_(0),
      response_size_(0) {}

void motis_con::query(std::string const& req, callback const& cb) {
  request_ = req;
  request_size_ = htonl(static_cast<uint32_t>(req.size()));

  connect([this, cb](net::tcp::tcp_ptr self, error_code ec) {
    if (ec) {
      return cb(self, "", ec);
    } else {
      return transfer(self, cb, ec);
    }
  });
}

#include "boost/asio/yield.hpp"
void motis_con::transfer(net::tcp::tcp_ptr self, callback cb, error_code ec) {
  if (ec) {
    boost::asio::detail::coroutine_ref(this) = 0;
    return respond(cb, self, ec);
  }

  if (!ec) {
    if (is_complete()) {
      boost::asio::detail::coroutine_ref(this) = 0;
    }

    using std::placeholders::_1;
    auto re = std::bind(&motis_con::transfer, this, self, cb, _1);
    error_code ignored;

    reenter(this) {
      // Write request size.
      yield asio::async_write(
          socket_,
          asio::buffer(reinterpret_cast<void*>(&request_size_),
                       sizeof(request_size_)),
          re);

      // Write request.
      yield asio::async_write(socket_, asio::buffer(request_),
                              asio::transfer_all(), re);

      // Read response size.
      yield asio::async_read(
          socket_,
          asio::buffer(reinterpret_cast<void*>(&response_size_),
                       sizeof(response_size_)),
          re);
      response_size_ = ntohl(response_size_);

      // Read response.
      yield asio::async_read(socket_, buf_,
                             asio::transfer_exactly(response_size_), re);
      copy_content(buf_.size());

      socket_.shutdown(asio::ip::tcp::socket::shutdown_send, ignored);
      return respond(cb, self, ec);
    }
  }
}
#include "boost/asio/unyield.hpp"

void motis_con::respond(callback const& cb, net::tcp::tcp_ptr self,
                        error_code ec) {
  finally(ec);
  if (asio::error::eof == ec) {
    ec = error_code();
  }
  std::string response(response_.begin(), response_.end());
  response_.clear();
  cb(std::move(self), response, ec);
}

std::size_t motis_con::copy_content(std::size_t buffer_size) {
  if (buffer_size > 0) {
    auto const buf = asio::buffer_cast<const char*>(buf_.data());
    std::size_t original = response_.size();
    response_.resize(response_.size() + buffer_size);
    std::memcpy(&(response_[original]), buf, buffer_size);
    buf_.consume(buffer_size);
  }
  return buffer_size;
}

}  // namespace client
}  // namespace motis
