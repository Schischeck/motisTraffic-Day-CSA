#pragma once

#include "boost/asio/deadline_timer.hpp"
#include "boost/system/error_code.hpp"

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct live_mode : public base_mode {
  live_mode(config* conf) : base_mode(conf) {}

  void init(motis::module::registry&) override;
  void init_async() override;

private:
  void schedule_update(boost::system::error_code);
  void handle_upload(motis::module::msg_ptr const&);
  void parse_zips();

  std::unique_ptr<boost::asio::deadline_timer> timer_;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
