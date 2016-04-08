#include "motis/launcher/batch_mode.h"

#include <memory>
#include <functional>
#include <algorithm>
#include <istream>
#include <ostream>
#include <fstream>

#include "motis/module/message.h"

using namespace motis::module;
namespace p = std::placeholders;

namespace motis {
namespace launcher {

std::string strip_new_lines(std::string s) {
  s.erase(std::remove(begin(s), end(s), '\n'), end(s));
  return s;
}

struct query_injector : std::enable_shared_from_this<query_injector> {
public:
  query_injector(boost::asio::io_service& ios,
                 motis::module::receiver& receiver,
                 std::string const& input_file_path,
                 std::string const& output_file_path)
      : ios_(ios),
        receiver_(receiver),
        in_(input_file_path),
        out_(output_file_path),
        next_query_id_(0) {
    in_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    out_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  }

  ~query_injector() { ios_.stop(); }

  void start() {
    auto self = shared_from_this();
    ios_.post([this, self]() {
      for (int i = 0; i < 10; ++i) inject_msg(self);
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

  void inject_msg(std::shared_ptr<query_injector>) {
    auto next = next_query();
    if (next) {
      receiver_.on_msg(
          next, std::bind(&query_injector::on_response, this,
                          shared_from_this(), ++next_query_id_, p::_1, p::_2));
    }
  }

  void on_response(std::shared_ptr<query_injector> self, int id, msg_ptr res,
                   boost::system::error_code ec) {
    write_response(id, res, ec);
    inject_msg(self);
  }

  void write_response(int id, msg_ptr res, boost::system::error_code ec) {
    msg_ptr response;

    if (ec) {
      MessageCreator b;
      b.CreateAndFinish(
          MsgContent_MotisError,
          CreateMotisError(b, ec.value(), b.CreateString(ec.category().name()),
                           b.CreateString(ec.message()))
              .Union());
      response = make_msg(b);
    } else if (res) {
      response = res;
    } else {
      MessageCreator b;
      b.CreateAndFinish(MsgContent_MotisSuccess, CreateMotisSuccess(b).Union());
      response = make_msg(b);
    }
    response->get()->mutate_id(id);

    out_ << strip_new_lines(response->to_json()) << "\n";
  }

  boost::asio::io_service& ios_;
  motis::module::receiver& receiver_;

  std::ifstream in_;
  std::ofstream out_;

  int next_query_id_;
};

void inject_queries(boost::asio::io_service& ios,
                    motis::module::receiver& receiver,
                    std::string const& input_file_path,
                    std::string const& output_file_path) {
  std::make_shared<query_injector>(ios, receiver, input_file_path,
                                   output_file_path)
      ->start();
}

}  // namespace launcher
}  // namespace motis
