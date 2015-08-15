#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/module/module.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/database.h"
#include "motis/realtime/message_fetcher.h"

namespace motis {
namespace realtime {

struct realtime : public motis::module::module {
  realtime();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "realtime"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override { return {}; }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  boost::asio::io_service ios_;
  std::unique_ptr<realtime_schedule> rts_;
  std::unique_ptr<delay_database> db_;
  std::unique_ptr<message_fetcher> message_fetcher_;

  // settings
  std::string db_server_, db_name_, db_user_, db_password_;
  std::vector<uint32_t> track_trains_;
  std::string load_msg_file_;
  bool debug_;
  std::time_t from_time_, to_time_;
  unsigned interval_;
};

}  // namespace realtime
}  // namespace motis
