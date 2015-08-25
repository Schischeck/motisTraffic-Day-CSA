#include "motis/realtime/realtime.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "boost/program_options.hpp"

#include "motis/module/api.h"
#include "motis/core/common/logging.h"

#include "motis/protocol/Message_generated.h"
#include "motis/protocol/RealtimeTrainInfoRequest_generated.h"
#include "motis/protocol/RealtimeTrainInfoResponse_generated.h"
#include "motis/protocol/RealtimeForwardTimeRequest_generated.h"

#include "motis/realtime/error.h"

#ifdef WITH_MYSQL
#include "motis/realtime/database.h"
#define DB_SERVER "realtime.db_server"
#define DB_NAME "realtime.db_name"
#define DB_USER "realtime.db_user"
#define DB_PASSWORD "realtime.db_password"
#define DB_FETCH_SIZE "realtime.db_fetch_size"
#endif

#define TRACK_TRAIN "realtime.track_train"
#define MSG_FILE "realtime.msg_file"
#define DEBUG_MODE "realtime.debug"
#define FROM_TIME "realtime.from"
#define TO_TIME "realtime.to"
#define LIVE "realtime.live"
#define MANUAL "realtime.manual"

using namespace motis::module;
using namespace motis::logging;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace realtime {

po::options_description realtime::desc() {
  po::options_description desc("Realtime Module");
#ifdef WITH_MYSQL
  desc.add_options()(
      DB_SERVER, po::value<std::string>(&db_server_)->default_value(db_server_),
      "MySQL host (e.g. localhost or 127.0.0.1)")(
      DB_NAME, po::value<std::string>(&db_name_)->default_value(db_name_),
      "MySQL database name (e.g. risr)")(
      DB_USER, po::value<std::string>(&db_user_)->default_value(db_user_),
      "MySQL user")(
      DB_PASSWORD,
      po::value<std::string>(&db_password_)->default_value(db_password_),
      "MySQL password")(DB_FETCH_SIZE, po::value<unsigned>(&db_fetch_size_),
                        "How many minutes should be loaded per db query");
#endif
  desc.add_options()(TRACK_TRAIN ",t",
                     po::value<std::vector<uint32_t>>(&track_trains_),
                     "Train number to track (debug)")(
      MSG_FILE, po::value<std::vector<std::string>>(&msg_files_),
      "Load realtime messages from a file")(
      DEBUG_MODE, po::bool_switch(&debug_)->default_value(false),
      "Enable lots of debug output")(
      FROM_TIME, po::value<opt_time>(&from_time_),
      "Load messages from " FROM_TIME
      " to " TO_TIME)(TO_TIME, po::value<opt_time>(&to_time_),
                      "Load messages from " FROM_TIME " to " TO_TIME)(
      LIVE, po::bool_switch(&live_)->default_value(false),
      "Automatically load new messages every minute")(
      MANUAL, po::bool_switch(&manual_)->default_value(false),
      "Don't load any messages automatically");
  return desc;
}

void realtime::print(std::ostream& out) const {
#ifdef WITH_MYSQL
  out << "  " << DB_SERVER << ": " << db_server_ << "\n"
      << "  " << DB_NAME << ": " << db_name_ << "\n"
      << "  " << DB_USER << ": " << db_user_ << "\n"
      << "  " << DB_PASSWORD << ": " << db_password_ << "\n"
      << "  " << DB_FETCH_SIZE << ": " << db_fetch_size_ << "\n";
#endif
  out << "  " << TRACK_TRAIN << ": ";
  std::copy(track_trains_.begin(), track_trains_.end(),
            std::ostream_iterator<uint32_t>(out, " "));
  out << "\n  " << MSG_FILE << ": ";
  std::copy(msg_files_.begin(), msg_files_.end(),
            std::ostream_iterator<std::string>(out, " "));
  out << "\n  " << DEBUG_MODE << ": " << debug_ << "\n"
      << "  " << FROM_TIME << ": " << from_time_ << "\n"
      << "  " << TO_TIME << ": " << to_time_ << "\n"
      << "  " << LIVE << ": " << live_ << "\n"
      << "  " << MANUAL << ": " << manual_;
}

realtime::realtime()
    : db_fetch_size_(15),
      ops_{{MsgContent_RealtimeTrainInfoRequest,
            std::bind(&realtime::get_train_info, this, p::_1, p::_2)},
           {MsgContent_RealtimeForwardTimeRequest,
            std::bind(&realtime::forward_time, this, p::_1, p::_2)}} {}

void realtime::init() {
  //
}

void realtime::on_config_loaded() {
  rts_ = std::unique_ptr<realtime_schedule>(
      new realtime_schedule(synced_sched<schedule_access::RW>().sched()));
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

  {
    std::ofstream f("stats.csv", std::ofstream::trunc);
    rts_->_stats.write_csv_header(f);
  }

  if (msg_files_.empty()) {
#ifdef WITH_MYSQL
    if (!db_name_.empty()) {
      auto db = std::unique_ptr<delay_database>(
          new delay_database(db_name_, db_server_, db_user_, db_password_));
      std::cout << "Connecting to DB..." << std::endl;
      if (!db->connect()) {
        std::cout << "Connection failed!" << std::endl;
        return;
      }
      auto msg_stream = std::unique_ptr<database_message_stream>(
          new database_message_stream(*rts_, std::move(db)));
      msg_stream->set_max_fetch_size(db_fetch_size_);
      std::time_t t1 = from_time_;
      if (t1 == 0 && !manual_) t1 = std::time(nullptr) - 60 * 1;
      std::time_t t2 = to_time_;
      if (t2 == 0 && !manual_) t2 = std::time(nullptr);
      msg_stream->start_at(t1);
      msg_stream->forward_to(t2);
      message_fetcher_.reset(
          new message_fetcher(*rts_, std::move(msg_stream), ios_));
    }
#endif
  } else {
    auto msg_stream = std::unique_ptr<file_message_stream>(
        new file_message_stream(*rts_, msg_files_));
    if (from_time_ != 0) {
      msg_stream->start_at(from_time_);
    }
    if (to_time_ != 0) {
      msg_stream->forward_to(to_time_);
    } else if (!manual_) {
      msg_stream->forward_to(std::time(nullptr));
    }
    message_fetcher_.reset(
        new message_fetcher(*rts_, std::move(msg_stream), ios_));
  }

  if (message_fetcher_ != nullptr) {
    if (!manual_) {
      message_fetcher_->process_stream();
    }
    message_fetcher_->set_live(live_);
    // TODO: live loop does not work (ios)
  }

  std::cout << "\nRealtime module initialized" << std::endl;
}

TimestampReason encode_reason(timestamp_reason reason) {
  switch (reason) {
    case timestamp_reason::SCHEDULE:
      return TimestampReason_Schedule;
    case timestamp_reason::IS:
    case timestamp_reason::REPAIR:
      return TimestampReason_Is;
    case timestamp_reason::FORECAST:
      return TimestampReason_Forecast;
    case timestamp_reason::PROPAGATION:
      return TimestampReason_Propagation;
  }
}

void realtime::get_train_info(motis::module::msg_ptr msg,
                              motis::module::callback cb) {
  auto req = msg->content<RealtimeTrainInfoRequest const*>();
  auto first_stop = req->first_stop();

  graph_event first_event(first_stop->station_index(), first_stop->train_nr(),
                          first_stop->departure(), first_stop->real_time(), -1);

  motis::node* route_node;
  motis::light_connection* lc;
  std::tie(route_node, lc) = rts_->locate_event(first_event);

  if (route_node == nullptr || lc == nullptr) {
    cb({}, error::event_not_found);
    return;
  }

  flatbuffers::FlatBufferBuilder b;
  std::vector<flatbuffers::Offset<EventInfo>> event_infos;

  while (route_node != nullptr && lc != nullptr) {
    motis::node* next_node = rts_->get_next_node(route_node);
    auto train_nr = lc->_full_con->con_info->train_nr;
    graph_event g_dep(route_node->get_station()->_id, train_nr, true,
                      lc->d_time, route_node->_route);
    graph_event g_arr(next_node->get_station()->_id, train_nr, false,
                      lc->a_time, route_node->_route);

    delay_info* di_dep = rts_->_delay_info_manager.get_delay_info(g_dep);
    delay_info* di_arr = rts_->_delay_info_manager.get_delay_info(g_arr);

    motis::time sched_dep = di_dep == nullptr
                                ? g_dep._current_time
                                : di_dep->_schedule_event._schedule_time;
    motis::time sched_arr = di_arr == nullptr
                                ? g_arr._current_time
                                : di_arr->_schedule_event._schedule_time;

    timestamp_reason reason_dep =
        di_dep == nullptr ? timestamp_reason::SCHEDULE : di_dep->_reason;
    timestamp_reason reason_arr =
        di_arr == nullptr ? timestamp_reason::SCHEDULE : di_arr->_reason;

    auto ei_dep = CreateEventInfo(b, train_nr, g_dep._station_index, true,
                                  g_dep._current_time, sched_dep,
                                  encode_reason(reason_dep));

    auto ei_arr = CreateEventInfo(b, train_nr, g_arr._station_index, false,
                                  g_arr._current_time, sched_arr,
                                  encode_reason(reason_arr));

    event_infos.push_back(ei_dep);
    event_infos.push_back(ei_arr);

    route_node = next_node;
    motis::edge* next_edge = rts_->get_next_edge(next_node);
    if (next_edge != nullptr) {
      lc = rts_->get_connection_with_service(next_edge, lc->a_time,
                                             lc->_full_con->con_info->service);
    } else {
      lc = nullptr;
    }
  }

  b.Finish(CreateMessage(
      b, MsgContent_RealtimeTrainInfoResponse,
      CreateRealtimeTrainInfoResponse(b, b.CreateVector(event_infos)).Union()));
  cb(make_msg(b), error::ok);
}

void realtime::forward_time(motis::module::msg_ptr msg,
                            motis::module::callback cb) {
  auto req = msg->content<RealtimeForwardTimeRequest const*>();

  std::time_t new_time = static_cast<std::time_t>(req->new_time());
  if (message_fetcher_ != nullptr) {
    if (message_fetcher_->_msg_stream->forward_to(new_time)) {
      message_fetcher_->process_stream();
      cb({}, error::ok);
    } else {
      cb({}, error::invalid_time);
    }
  } else {
    cb({}, error::no_message_stream);
  }
}

void realtime::on_msg(msg_ptr msg, sid, callback cb) {
  auto it = ops_.find(msg->msg_->content_type());
  it->second(msg, cb);
}

MOTIS_MODULE_DEF_MODULE(realtime)

}  // namespace realtime
}  // namespace motis
