#include "motis/realtime/realtime.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "boost/program_options.hpp"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/realtime/error.h"
#include "motis/realtime/handler/addition_handler.h"
#include "motis/realtime/handler/cancel_handler.h"
#include "motis/realtime/handler/connection_assessment_handler.h"
#include "motis/realtime/handler/connection_decision_handler.h"
#include "motis/realtime/handler/delay_handler.h"
#include "motis/realtime/handler/reroute_handler.h"
#include "motis/realtime/realtime_context.h"

// options
#define TRACK_TRAIN "realtime.track_train"
#define LOAD_MSG_FILE "realtime.load_msg_file"
#define DEBUG_MODE "realtime.debug"

using namespace motis::module;
using namespace motis::realtime::handler;
using namespace motis::ris;
namespace po = boost::program_options;

namespace motis {
namespace realtime {

po::options_description realtime::desc() {
  po::options_description desc("Realtime Module");
  // clang-format off
  desc.add_options()(
      TRACK_TRAIN ",t",
      po::value<std::vector<uint32_t>>(&track_trains_),
      "Train number to track (debug)")(
      LOAD_MSG_FILE, po::value<std::string>(&load_msg_file_),
      "Load realtime messages from a file (disables database)")(
      DEBUG_MODE, po::bool_switch(&debug_)->default_value(false),
      "Enable lots of debug output");
  // clang-format on
  return desc;
}

void realtime::print(std::ostream& out) const {
  std::copy(track_trains_.begin(), track_trains_.end(),
            std::ostream_iterator<uint32_t>(out, " "));
  out << "\n  " << LOAD_MSG_FILE << ": " << load_msg_file_ << "\n"
      << "  " << DEBUG_MODE << ": " << debug_;
}

void realtime::init() {
  // rts_ = std::unique_ptr<realtime_schedule>(
  //     new realtime_schedule(synced_sched<schedule_access::RW>().sched()));
  // rts_->_debug_mode = debug_;

  // for (auto t : track_trains_) {
  //   rts_->track_train(t);
  // }
  // if (!rts_->_tracked_trains.empty()) {
  //   std::cerr << std::endl;
  //   std::cout << std::endl;
  //   std::cout << "Tracking " << rts_->_tracked_trains.size() << " trains"
  //             << std::endl;
  // }

  // if (!load_msg_file_.empty()) {
  //   std::cout << "Loading messages from " << load_msg_file_ << "..."
  //             << std::endl;
  //   std::ifstream f(load_msg_file_);
  //   rts_->_message_handler.process_message_stream(f);
  // }

  // db_ = std::unique_ptr<delay_database>(
  //     new delay_database(db_name_, db_server_, db_user_, db_password_));
  // message_fetcher_ =
  //     std::unique_ptr<message_fetcher>(new message_fetcher(*rts_, *db_,
  //     ios_));

  // {
  //   std::ofstream f("stats.csv", std::ofstream::trunc);
  //   rts_->_stats.write_csv_header(f);
  // }

  // if (!db_name_.empty() && load_msg_file_.empty()) {
  //   std::cout << "Connecting to DB..." << std::endl;
  //   if (!db_->connect()) {
  //     std::cout << "Connection failed!" << std::endl;
  //     return;
  //   }

  //   std::time_t t1 = from_time_;
  //   if (t1 == 0) t1 = std::time(nullptr) - 60 * 1;
  //   std::time_t t2 = to_time_;
  //   if (t2 == 0) t2 = std::time(nullptr);
  //   message_fetcher_->load(t1, t2, interval_);

  //   if (to_time_ == 0) {
  //     message_fetcher_->start_loop();
  //   } else {
  //     std::ofstream f("stats.csv", std::ofstream::app);
  //     rts_->_stats.write_csv(f, t1, t2);
  //   }
  // }
}

void realtime::on_msg(msg_ptr msg, sid, callback cb) {
  auto req = msg->content<motis::ris::RISBatch const*>();

  realtime_context ctx;

  for (auto const& holder : *req->messages()) {
    auto const& nested = holder->message_nested_root();
    auto const& msg = nested->content();

    switch (nested->content_type()) {
      case MessageUnion_DelayMessage:
        handle_delay(reinterpret_cast<DelayMessage const*>(msg), ctx);
        break;
      case MessageUnion_CancelMessage:
        handle_cancel(reinterpret_cast<CancelMessage const*>(msg), ctx);
        break;
      case MessageUnion_AdditionMessage:
        handle_addition(reinterpret_cast<AdditionMessage const*>(msg), ctx);
        break;
      case MessageUnion_RerouteMessage:
        handle_reroute(reinterpret_cast<RerouteMessage const*>(msg), ctx);
        break;
      case MessageUnion_ConnectionDecisionMessage:
        handle_connection_decision(
            reinterpret_cast<ConnectionDecisionMessage const*>(msg), ctx);
        break;
      case MessageUnion_ConnectionAssessmentMessage:
        handle_connection_assessment(
            reinterpret_cast<ConnectionAssessmentMessage const*>(msg), ctx);
        break;
    };
  }
  return cb({}, error::ok);
}

}  // namespace realtime
}  // namespace motis
