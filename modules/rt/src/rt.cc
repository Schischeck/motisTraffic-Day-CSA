#include "motis/rt/rt.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/access/time_access.h"
#include "motis/rt/handler/addition_handler.h"
#include "motis/rt/handler/cancel_handler.h"
#include "motis/rt/handler/connection_assessment_handler.h"
#include "motis/rt/handler/connection_decision_handler.h"
#include "motis/rt/handler/context.h"
#include "motis/rt/handler/delay_handler.h"
#include "motis/rt/handler/reroute_handler.h"
#include "motis/rt/error.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::rt::handler;
using namespace motis::ris;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace rt {

po::options_description rt::desc() {
  po::options_description desc{"rt Module"};
  return desc;
}

void rt::on_msg(msg_ptr msg, sid, callback cb) {
  auto req = msg->content<motis::ris::RISBatch const*>();
  auto lock = synced_sched<schedule_access::RW>();
  auto const& sched = lock.sched();

  std::cout << "\n--" << std::endl;
  for (auto const& trip : sched.trip_mem) {
    std::cout << sched.stations[trip->id.primary.station_id]->eva_nr << " "
              << trip->id.primary.train_nr << " "
              << motis_to_unixtime(sched, trip->id.primary.time) << std::endl;
  }

  handler::context ctx{lock.sched(), {}};  // TODO
  unsigned long exceptions = 0;

  for (auto const& holder : *req->messages()) {
    auto const& nested = holder->message_nested_root();
    auto const& msg = nested->content();

    try {
      // rts_->_stats._counters.messages.increment();
      switch (nested->content_type()) {
        case MessageUnion_DelayMessage:
          handle_delay(ctx, reinterpret_cast<DelayMessage const*>(msg));
          break;
        case MessageUnion_CancelMessage:
          handle_cancel(ctx, reinterpret_cast<CancelMessage const*>(msg));
          break;
        case MessageUnion_AdditionMessage:
          handle_addition(ctx, reinterpret_cast<AdditionMessage const*>(msg));
          break;
        case MessageUnion_RerouteMessage:
          handle_reroute(ctx, reinterpret_cast<RerouteMessage const*>(msg));
          break;
        case MessageUnion_ConnectionDecisionMessage:
          handle_connection_decision(
              ctx, reinterpret_cast<ConnectionDecisionMessage const*>(msg));
          break;
        case MessageUnion_ConnectionAssessmentMessage:
          handle_connection_assessment(
              ctx, reinterpret_cast<ConnectionAssessmentMessage const*>(msg));
          break;
        default:
          // rts_->_stats._counters.unknown.increment();
          // rts_->_stats._counters.unknown.ignore();
          break;
      }
    } catch (...) {
      // TODO
      ++exceptions;
    }
  }

  std::cout << "\ntotal: " << req->messages()->size()
            << "\nlookups: " << ctx.stats.trip_lookups
            << "\nds1100: " << ctx.stats.ds100
            << "\nfound: " << ctx.stats.found_trips
            << "\nmissed primary: " << ctx.stats.missed_primary
            << " (no train_nr didnt help " << ctx.stats.no_train_nr_didnt_help
            << " / pair miss " << ctx.stats.station_train_nr_miss << ")"
            << "\nmissed secondary: " << ctx.stats.missed_secondary
            << "\nexceptions: " << exceptions << std::endl;
  return cb({}, error::ok);
}

}  // namespace rt
}  // namespace motis
