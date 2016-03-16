#include "motis/rt/rt.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>

#include "boost/program_options.hpp"

#include "motis/rt/handler/addition_handler.h"
#include "motis/rt/handler/cancel_handler.h"
#include "motis/rt/handler/connection_assessment_handler.h"
#include "motis/rt/handler/connection_decision_handler.h"
#include "motis/rt/handler/context.h"
#include "motis/rt/handler/delay_handler.h"
#include "motis/rt/handler/reroute_handler.h"

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
  // auto sched = synced_sched<schedule_access::RW>();

  handler::context ctx;

  for (auto const& holder : *req->messages()) {
    auto const& nested = holder->message_nested_root();
    auto const& msg = nested->content();

    // rts_->_stats._counters.messages.increment();
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
        // rts_->_stats._counters.unknown.increment();
        // rts_->_stats._counters.unknown.ignore();
        break;
    };
  }
}

}  // namespace rt
}  // namespace motis
