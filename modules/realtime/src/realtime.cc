#include "motis/realtime/realtime.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "boost/program_options.hpp"

#include "motis/module/api.h"
#include "motis/core/common/logging.h"

// options
#define DB_SERVER "realtime.db_server"
#define DB_NAME "realtime.db_name"
#define DB_USER "realtime.db_user"
#define DB_PASSWORD "realtime.db_password"
#define TRACK_TRAIN "realtime.track_train"
#define LOAD_MSG_FILE "realtime.load_msg_file"
#define DEBUG_MODE "realtime.debug"
#define FROM_TIME "realtime.from"
#define TO_TIME "realtime.to"
#define INTERVAL "realtime.interval"

using namespace json11;
using namespace motis::module;
using namespace motis::logging;
namespace po = boost::program_options;

namespace motis {
namespace realtime {

po::options_description realtime::desc() {
  po::options_description desc("Realtime Module");
  desc.add_options()(
      DB_SERVER, po::value<std::string>(&db_server_)->default_value(db_server_),
      "MySQL host (e.g. localhost or 127.0.0.1)")(
      DB_NAME, po::value<std::string>(&db_name_)->default_value(db_name_),
      "MySQL database name (e.g. risr)")(
      DB_USER, po::value<std::string>(&db_user_)->default_value(db_user_),
      "MySQL user")(DB_PASSWORD, po::value<std::string>(&db_password_)
                                     ->default_value(db_password_),
                    "MySQL password")(
      TRACK_TRAIN ",t", po::value<std::vector<uint32_t>>(&track_trains_),
      "Train number to track (debug)")(
      LOAD_MSG_FILE, po::value<std::string>(&load_msg_file_),
      "Load realtime messages from a file (disables database)")(
      DEBUG_MODE, po::bool_switch(&debug_)->default_value(false),
      "Enable lots of debug output")(
      FROM_TIME, po::value<std::time_t>(&from_time_),
      "Load messages from " FROM_TIME
      " to " TO_TIME)(TO_TIME, po::value<std::time_t>(&to_time_),
                      "Load messages from " FROM_TIME " to " TO_TIME)(
      INTERVAL, po::value<unsigned>(&interval_),
      "How many minutes should be loaded per update");
  return desc;
}

void realtime::print(std::ostream& out) const {
  out << "  " << DB_SERVER << ": " << db_server_ << "\n"
      << "  " << DB_NAME << ": " << db_name_ << "\n"
      << "  " << DB_USER << ": " << db_user_ << "\n"
      << "  " << DB_PASSWORD << ": " << db_password_ << "\n"
      << "  " << TRACK_TRAIN << ": ";
  std::copy(track_trains_.begin(), track_trains_.end(),
            std::ostream_iterator<uint32_t>(out, " "));
  out << "\n  " << LOAD_MSG_FILE << ": " << load_msg_file_ << "\n"
      << "  " << DEBUG_MODE << ": " << debug_ << "\n"
      << "  " << FROM_TIME << ": " << from_time_ << "\n"
      << "  " << TO_TIME << ": " << to_time_ << "\n"
      << "  " << INTERVAL << ": " << interval_;
}

realtime::realtime() : ops_{}, from_time_(0), to_time_(0), interval_(1) {}

void realtime::init() {
  rts_ = std::unique_ptr<realtime_schedule>(new realtime_schedule(*schedule_));
  rts_->_debug_mode = debug_;

  for (auto t : track_trains_) {
    rts_->track_train(t);
  }
  if (!rts_->_tracked_trains.empty()) {
    std::cerr << std::endl;
    std::cout << std::endl;
    std::cout << "Tracking " << rts_->_tracked_trains.size() << " trains"
              << std::endl;
  }

  if (!load_msg_file_.empty()) {
    std::cout << "Loading messages from " << load_msg_file_ << "..."
              << std::endl;
    std::ifstream f(load_msg_file_);
    rts_->_message_handler.process_message_stream(f);
  }

  db_ = std::unique_ptr<delay_database>(
      new delay_database(db_name_, db_server_, db_user_, db_password_));
  message_fetcher_ =
      std::unique_ptr<message_fetcher>(new message_fetcher(*rts_, *db_, ios_));

  {
    std::ofstream f("stats.csv", std::ofstream::trunc);
    rts_->_stats.write_csv_header(f);
  }

  if (!db_name_.empty() && load_msg_file_.empty()) {
    std::cout << "Connecting to DB..." << std::endl;
    if (!db_->connect()) {
      std::cout << "Connection failed!" << std::endl;
      return;
    }

    std::time_t t1 = from_time_;
    if (t1 == 0) t1 = std::time(nullptr) - 60 * 1;
    std::time_t t2 = to_time_;
    if (t2 == 0) t2 = std::time(nullptr);
    message_fetcher_->load(t1, t2, interval_);

    if (to_time_ == 0) {
      message_fetcher_->start_loop();
    } else {
      std::ofstream f("stats.csv", std::ofstream::app);
      rts_->_stats.write_csv(f, t1, t2);
    }
  }
}

Json realtime::on_msg(Json const& msg, sid) {
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  return op->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(realtime)

}  // namespace realtime
}  // namespace motis
