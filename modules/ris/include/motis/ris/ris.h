#pragma once

#include <string>

#include "boost/system/error_code.hpp"
#include "boost/asio/deadline_timer.hpp"

#include "motis/module/module.h"

namespace po = boost::program_options;

namespace motis {
namespace ris {

struct ris : public motis::module::module {
  ris();

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  void init() override;
  std::string name() const override { return "ris"; }
  std::vector<MsgContent> subscriptions() const override { return {}; }
  void on_msg(motis::module::msg_ptr, motis::module::sid,
              motis::module::callback) override {}

private:
  void parse_zips();
  void schedule_update(boost::system::error_code e);

  int update_interval_;
  std::string zip_folder_;
  std::unique_ptr<boost::asio::deadline_timer> timer_;
};

}  // namespace ris
}  // namespace motis
