#include "motis/realtime/realtime.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>

#include "boost/program_options.hpp"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/logging.h"
#include "motis/realtime/error.h"
#include "motis/realtime/handler/addition_handler.h"
#include "motis/realtime/handler/cancel_handler.h"
#include "motis/realtime/handler/connection_assessment_handler.h"
#include "motis/realtime/handler/connection_decision_handler.h"
#include "motis/realtime/handler/delay_handler.h"
#include "motis/realtime/handler/reroute_handler.h"
#include "motis/realtime/realtime_context.h"
#include "motis/realtime/statistics.h"

// options
#define TRACK_TRAIN "realtime.track_train"
#define DEBUG_MODE "realtime.debug"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::realtime::handler;
using namespace motis::ris;
using namespace motis::logging;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace realtime {

realtime::realtime()
    : debug_(false),
      ops_{{MsgContent_RealtimeTrainInfoRequest,
            std::bind(&realtime::get_train_info, this, p::_1, p::_2)},
           {MsgContent_RealtimeTrainInfoBatchRequest,
            std::bind(&realtime::get_batch_train_info, this, p::_1, p::_2)},
           {MsgContent_RealtimeCurrentTimeRequest,
            std::bind(&realtime::get_current_time, this, p::_1, p::_2)},
           {MsgContent_RealtimeDelayInfoRequest,
            std::bind(&realtime::get_delay_infos, this, p::_1, p::_2)},
           {MsgContent_RISBatch,
            std::bind(&realtime::handle_ris_msgs, this, p::_1, p::_2)}} {}

po::options_description realtime::desc() {
  po::options_description desc("Realtime Module");
  // clang-format off
   desc.add_options()
       (TRACK_TRAIN ",t", po::value<std::vector<uint32_t>>(&track_trains_),
       "Train number to track (debug)")
       (DEBUG_MODE, po::bool_switch(&debug_)->default_value(false),
       "Enable lots of debug output");
  // clang-format on
  return desc;
}

void realtime::print(std::ostream& out) const {
  std::copy(track_trains_.begin(), track_trains_.end(),
            std::ostream_iterator<uint32_t>(out, " "));
  out << "\n  " << DEBUG_MODE << ": " << debug_;
}

void realtime::init() {
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

  std::ofstream f("stats.csv", std::ofstream::trunc);
  rts_->_stats.write_csv_header(f);
}

void realtime::on_msg(msg_ptr msg, sid, callback cb) {
  auto it = ops_.find(msg->msg_->content_type());
  it->second(msg, cb);  // XXX error handling
}

TimestampReason encode_reason(timestamp_reason reason) {
  switch (reason) {
    case timestamp_reason::SCHEDULE: return TimestampReason_Schedule;
    case timestamp_reason::IS:
    case timestamp_reason::REPAIR: return TimestampReason_Is;
    case timestamp_reason::FORECAST: return TimestampReason_Forecast;
    case timestamp_reason::PROPAGATION: return TimestampReason_Propagation;
  }
  return TimestampReason_Propagation;
}

Offset<RealtimeTrainInfoResponse> build_train_info_response(
    FlatBufferBuilder& b, realtime_schedule* rts,
    RealtimeTrainInfoRequest const* req) {
  auto first_stop = req->first_stop();

  graph_event first_event(first_stop->station_index(), first_stop->train_nr(),
                          first_stop->departure(), first_stop->real_time(),
                          first_stop->route_id());

  motis::node* route_node;
  motis::light_connection* lc;
  std::tie(route_node, lc) = rts->locate_event(first_event);

  if (route_node == nullptr || lc == nullptr) {
    throw boost::system::system_error(error::event_not_found);
  }

  first_event._route_id = route_node->_route;

  std::vector<Offset<EventInfo>> event_infos;
  if (first_event.arrival()) {
    delay_info* di_arr = rts->_delay_info_manager.get_delay_info(first_event);
    motis::time sched_arr = di_arr == nullptr
                                ? first_event._current_time
                                : di_arr->_schedule_event._schedule_time;
    timestamp_reason reason_arr =
        di_arr == nullptr ? timestamp_reason::SCHEDULE : di_arr->_reason;
    auto ei_arr = CreateEventInfo(
        b, lc->_full_con->con_info->train_nr, first_event._station_index, false,
        first_event._current_time, sched_arr, encode_reason(reason_arr));
    event_infos.push_back(ei_arr);

    if (req->single_event()) {
      lc = nullptr;
    } else {
      motis::edge* next_edge = rts->get_next_edge(route_node);
      if (next_edge != nullptr) {
        lc = next_edge->get_connection(lc->a_time);
      } else {
        lc = nullptr;
      }
    }
  }

  while (route_node != nullptr && lc != nullptr) {
    auto train_nr = lc->_full_con->con_info->train_nr;

    graph_event g_dep(route_node->get_station()->_id, train_nr, true,
                      lc->d_time, route_node->_route);
    delay_info* di_dep = rts->_delay_info_manager.get_delay_info(g_dep);
    motis::time sched_dep = di_dep == nullptr
                                ? g_dep._current_time
                                : di_dep->_schedule_event._schedule_time;
    timestamp_reason reason_dep =
        di_dep == nullptr ? timestamp_reason::SCHEDULE : di_dep->_reason;
    auto ei_dep = CreateEventInfo(b, train_nr, g_dep._station_index, true,
                                  g_dep._current_time, sched_dep,
                                  encode_reason(reason_dep));
    event_infos.push_back(ei_dep);

    if (req->single_event()) {
      break;
    }

    motis::node* next_node = rts->get_next_node(route_node);
    graph_event g_arr(next_node->get_station()->_id, train_nr, false,
                      lc->a_time, route_node->_route);
    delay_info* di_arr = rts->_delay_info_manager.get_delay_info(g_arr);
    motis::time sched_arr = di_arr == nullptr
                                ? g_arr._current_time
                                : di_arr->_schedule_event._schedule_time;
    timestamp_reason reason_arr =
        di_arr == nullptr ? timestamp_reason::SCHEDULE : di_arr->_reason;
    auto ei_arr = CreateEventInfo(b, train_nr, g_arr._station_index, false,
                                  g_arr._current_time, sched_arr,
                                  encode_reason(reason_arr));
    event_infos.push_back(ei_arr);

    route_node = next_node;
    motis::edge* next_edge = rts->get_next_edge(next_node);
    if (next_edge != nullptr) {
      lc = next_edge->get_connection(lc->a_time);
    } else {
      lc = nullptr;
    }
  }

  return CreateRealtimeTrainInfoResponse(b, b.CreateVector(event_infos),
                                         first_event._route_id);
}

void realtime::get_train_info(msg_ptr msg, callback cb) {
  auto sched = synced_sched<schedule_access::RO>();
  auto req = msg->content<RealtimeTrainInfoRequest const*>();

  try {
    MessageCreator b;
    b.CreateAndFinish(MsgContent_RealtimeTrainInfoResponse,
                      build_train_info_response(b, rts_.get(), req).Union());
    return cb(make_msg(b), error::ok);
  } catch (boost::system::system_error const& e) {
    return cb({}, e.code());
  }
}

void realtime::get_batch_train_info(msg_ptr msg, callback cb) {
  auto sched = synced_sched<schedule_access::RO>();
  auto requests = msg->content<RealtimeTrainInfoBatchRequest const*>();

  MessageCreator b;
  std::vector<Offset<RealtimeTrainInfoResponse>> responses;

  try {
    for (auto req : *requests->trains()) {
      responses.push_back(build_train_info_response(b, rts_.get(), req));
    }
  } catch (boost::system::system_error const& e) {
    return cb({}, e.code());
  }

  b.CreateAndFinish(
      MsgContent_RealtimeTrainInfoBatchResponse,
      CreateRealtimeTrainInfoBatchResponse(b, b.CreateVector(responses))
          .Union());
  return cb(make_msg(b), error::ok);
}

void realtime::get_current_time(msg_ptr, callback) {
  throw std::runtime_error("realtime::get_current_time is not implemented.");
}

InternalTimestampReason encode_internal_reason(timestamp_reason reason) {
  switch (reason) {
    case timestamp_reason::SCHEDULE: return InternalTimestampReason_Schedule;
    case timestamp_reason::IS: return InternalTimestampReason_Is;
    case timestamp_reason::REPAIR: return InternalTimestampReason_Repair;
    case timestamp_reason::FORECAST: return InternalTimestampReason_Forecast;
    case timestamp_reason::PROPAGATION:
      return InternalTimestampReason_Propagation;
  }
  return InternalTimestampReason_Propagation;
}

void pack_delay_infos(MessageCreator& fbb, std::vector<delay_info*>& dis) {
  std::sort(std::begin(dis), std::end(dis), [](delay_info* a, delay_info* b) {
    const auto& ae = a->_schedule_event;
    const auto& be = b->_schedule_event;
    return std::tie(ae._schedule_time, ae._station_index, ae._train_nr,
                    ae._departure) < std::tie(be._schedule_time,
                                              be._station_index, be._train_nr,
                                              be._departure);
  });

  std::vector<flatbuffers::Offset<DelayInfo>> delay_infos;
  delay_infos.reserve(dis.size());
  for (const delay_info* di : dis) {
    DelayInfoBuilder dib(fbb);
    dib.add_train_nr(di->_schedule_event._train_nr);
    dib.add_station_index(di->_schedule_event._station_index);
    dib.add_departure(di->_schedule_event.departure());
    dib.add_scheduled_time(di->_schedule_event._schedule_time);
    dib.add_current_time(di->_current_time);
    dib.add_forecast_time(di->_forecast_time);
    dib.add_canceled(di->_canceled);
    dib.add_reason(encode_internal_reason(di->_reason));
    dib.add_route_id(di->_route_id);
    delay_infos.push_back(dib.Finish());
  }

  fbb.CreateAndFinish(
      MsgContent_RealtimeDelayInfoResponse,
      CreateRealtimeDelayInfoResponse(fbb, fbb.CreateVector(delay_infos))
          .Union());
}

void realtime::get_delay_infos(msg_ptr, callback cb) {
  auto const& dis = rts_->_delay_info_manager.delay_infos();

  std::vector<delay_info*> delay_infos;
  delay_infos.reserve(dis.size());
  for (auto const& di : dis) {
    delay_infos.push_back(di.get());
  }

  MessageCreator b;
  pack_delay_infos(b, delay_infos);
  cb(make_msg(b), error::ok);
}

void realtime::handle_ris_msgs(msg_ptr msg, callback cb) {
  auto req = msg->content<motis::ris::RISBatch const*>();
  auto sched = synced_sched<schedule_access::RW>();
  auto& ctx = *rts_;

  operation_timer timer(rts_->_stats._total_processing);
  statistics before_stats = rts_->_stats;

  for (auto const& holder : *req->messages()) {
    auto const& nested = holder->message_nested_root();
    auto const& msg = nested->content();

    rts_->_stats._counters.messages.increment();
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
      default:
        rts_->_stats._counters.unknown.increment();
        rts_->_stats._counters.unknown.ignore();
        break;
    };
  }

  rts_->_delay_propagator.process_queue();
  rts_->_graph_updater.finish_graph_update();

  statistics run_stats = rts_->_stats - before_stats;
  std::cout << "this run:\n";
  run_stats.print(std::cout);
  std::cout << "total so far:\n";
  rts_->_stats.print(std::cout);

  {
    std::ofstream f("stats.csv", std::ofstream::app);
    run_stats.write_csv(f, /*start_time*/ 0, /*end_time*/ 0);
  }

  MessageCreator b;
  auto delay_infos = rts_->_delay_info_manager.get_delay_info_delta();
  pack_delay_infos(b, delay_infos);
  dispatch(make_msg(b));

  return cb({}, error::ok);
}

}  // namespace realtime
}  // namespace motis
