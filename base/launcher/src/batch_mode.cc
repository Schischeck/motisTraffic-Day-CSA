#include "motis/launcher/batch_mode.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <istream>
#include <memory>
#include <ostream>

#include "utl/erase.h"

#include "motis/module/message.h"

using namespace motis::module;
namespace p = std::placeholders;

namespace motis {
namespace launcher {

struct query_injector : std::enable_shared_from_this<query_injector> {
public:
  query_injector(boost::asio::io_service& ios,
                 motis::module::receiver& receiver,
                 std::string const& input_file_path,
                 std::string const& output_file_path, int num_threads)
      : ios_(ios),
        receiver_(receiver),
        in_(input_file_path),
        out_(output_file_path),
        num_threads_(num_threads) {
    in_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    out_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  }

  query_injector(query_injector const&) = delete;
  query_injector& operator=(query_injector const&) = delete;

  query_injector(query_injector&&) = delete;
  query_injector& operator=(query_injector&&) = delete;

  ~query_injector() { ios_.stop(); }

  void start() {
    auto self = shared_from_this();
    ios_.post([this, self]() {
      for (int i = 0; i < 2 * num_threads_; ++i) {
        if (!inject_msg(self)) {
          break;
        }
      }
    });
  }

private:
  msg_ptr next_query() {
    if (in_.eof() || in_.peek() == EOF) {
      return nullptr;
    }

    std::string json;
    std::getline(in_, json);
    return make_msg(json);
  }

  bool inject_msg(std::shared_ptr<query_injector> const&) {
    msg_ptr next;
    try {
      next = next_query();
      if (next) {
        receiver_.on_msg(next, ios_.wrap(std::bind(&query_injector::on_response,
                                                   this, shared_from_this(),
                                                   next->id(), p::_1, p::_2)));
      } else {
        return false;
      }
    } catch (std::system_error const& e) {
      on_response(shared_from_this(), next ? next->id() : -1, msg_ptr(),
                  e.code());
    }
    return true;
  }

  void on_response(std::shared_ptr<query_injector> self, int id, msg_ptr res,
                   std::error_code ec) {
    write_response(id, std::move(res), ec);
    inject_msg(std::move(self));
  }

  void write_response(int id, msg_ptr const& res, std::error_code ec) {
    msg_ptr response;

    if (ec) {
      response = make_error_msg(ec);
    } else if (res) {
      response = res;
    } else {
      response = make_success_msg();
    }
    response->get()->mutate_id(id);

    auto json = response->to_json();
    utl::erase(json, '\n');

    out_ << json << "\n";
  }

  boost::asio::io_service& ios_;
  motis::module::receiver& receiver_;

  std::ifstream in_;
  std::ofstream out_;

  int num_threads_;
};

void inject_queries(boost::asio::io_service& ios,
                    motis::module::receiver& receiver,
                    std::string const& input_file_path,
                    std::string const& output_file_path, int num_threads) {
  std::make_shared<query_injector>(ios, receiver, input_file_path,
                                   output_file_path, num_threads)
      ->start();
}

}  // namespace launcher
}  // namespace motis
