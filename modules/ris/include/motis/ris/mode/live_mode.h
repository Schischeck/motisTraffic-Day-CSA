#pragma once

#include "boost/asio/deadline_timer.hpp"
#include "boost/system/error_code.hpp"

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct live_mode : public base_mode {

  live_mode(ris* module) : base_mode(module) {}

  virtual void init_async() override;
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  void schedule_update(boost::system::error_code e);
  void parse_zips();

  std::unique_ptr<boost::asio::deadline_timer> timer_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
