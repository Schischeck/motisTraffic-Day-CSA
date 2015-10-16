#pragma once

#include <string>

#include "boost/system/error_code.hpp"
#include "boost/asio/deadline_timer.hpp"

#include "motis/module/module.h"

namespace po = boost::program_options;

namespace motis {
namespace ris {

struct ris : public motis::module::module {
  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  void init() override;
  std::string name() override { return "ris"; }
  std::vector<MsgContent> subscriptions() override { return {}; }
  void on_msg(motis::module::msg_ptr msg, motis::module::sid session,
              motis::module::callback cb) override {}

private:
  void schedule_update(boost::system::error_code e);

  int update_interval_;
  std::string zip_folder_;
  std::unique_ptr<boost::asio::deadline_timer> timer_;
};

}  // namespace ris
}  // namespace motis
