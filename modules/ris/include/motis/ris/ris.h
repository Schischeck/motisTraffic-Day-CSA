#pragma once

#include <ctime>
#include <set>
#include <string>

#include "boost/system/error_code.hpp"
#include "boost/asio/deadline_timer.hpp"

#include "motis/module/module.h"

namespace po = boost::program_options;

namespace motis {
namespace ris {

enum class mode : bool { LIVE, SIMULATION };

struct ris : public motis::module::module {
  ris();
  virtual ~ris() {}

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  virtual void init_async() override;
  std::string name() const override { return "ris"; }
  std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RISForwardTimeRequest, MsgContent_HTTPRequest};
  }

  void on_msg(motis::module::msg_ptr, motis::module::sid,
              motis::module::callback) override;
  void handle_forward_time(motis::module::msg_ptr msg,
                           motis::module::callback cb);
  void handle_zipfile_upload(motis::module::msg_ptr msg,
                             motis::module::callback cb);

private:
  void fill_database();

  void schedule_update(boost::system::error_code e);
  void parse_zips();

  void forward_time(std::time_t new_time, motis::module::callback finished_cb);

  mode mode_;
  int update_interval_;
  std::string zip_folder_;
  int max_days_;
  std::time_t simulation_start_time_;

  std::time_t simulation_time_;
  std::unique_ptr<boost::asio::deadline_timer> timer_;
  std::set<std::string> read_files_;
};

}  // namespace ris
}  // namespace motis
