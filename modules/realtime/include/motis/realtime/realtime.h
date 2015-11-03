#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/module/module.h"

namespace motis {
namespace realtime {

struct realtime : public motis::module::module {

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "realtime"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RISBatch};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  boost::asio::io_service ios_;
  // std::unique_ptr<realtime_schedule> rts_;
  // std::unique_ptr<delay_database> db_;
  // std::unique_ptr<message_fetcher> message_fetcher_;

  // settings
  std::vector<uint32_t> track_trains_;
  std::string load_msg_file_;
  bool debug_;
};

}  // namespace realtime
}  // namespace motis
