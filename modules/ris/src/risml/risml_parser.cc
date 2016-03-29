#include "motis/ris/risml/risml_parser.h"

#include <map>

#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/logging.h"
#include "motis/ris/risml/common.h"

using namespace std::placeholders;
using namespace flatbuffers;
using namespace pugi;
using namespace parser;
using namespace motis::logging;

namespace motis {
namespace ris {
namespace risml {

using parser_func_t = std::function<Offset<Message>(context&, xml_node const&)>;

Offset<Message> parse_delay_msg(context& ctx, xml_node const& msg,
                                DelayType type) {
  std::vector<Offset<UpdatedEvent>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const&) {
    auto attr_name = (type == DelayType_Is) ? "Ist" : "Prog";
    auto updated = parse_time(child_attr(e_node, "Zeit", attr_name).value());
    events.push_back(CreateUpdatedEvent(ctx.b, event, updated));
  });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b, MessageUnion_DelayMessage,
      CreateDelayMessage(ctx.b, trip_id, type, ctx.b.CreateVector(events))
          .Union());
}

Offset<Message> parse_cancel_msg(context& ctx, xml_node const& msg) {
  std::vector<Offset<Event>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event, xml_node const&,
                              xml_node const&) { events.push_back(event); });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b, MessageUnion_CancelMessage,
      CreateCancelMessage(ctx.b, trip_id, ctx.b.CreateVector(events)).Union());
}

Offset<Message> parse_addition_msg(context& ctx, xml_node const& msg) {
  std::vector<Offset<AdditionalEvent>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const& t_node) {
    events.push_back(parse_additional_event(ctx.b, event, e_node, t_node));
  });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b, MessageUnion_AdditionMessage,
      CreateAdditionMessage(ctx.b, trip_id, ctx.b.CreateVector(events))
          .Union());
}

Offset<Message> parse_reroute_msg(context& ctx, xml_node const& msg) {
  std::vector<Offset<Event>> cancelled_events;
  foreach_event(ctx, msg,
                [&](Offset<Event> const& event, xml_node const&,
                    xml_node const&) { cancelled_events.push_back(event); });

  std::vector<Offset<ReroutedEvent>> new_events;
  foreach_event(
      ctx, msg,
      [&](Offset<Event> const& event, xml_node const& e_node,
          xml_node const& t_node) {
        auto additional = parse_additional_event(ctx.b, event, e_node, t_node);
        cstr status_str = e_node.attribute("RegSta").value();
        auto status = (status_str == "Normal") ? RerouteStatus_Normal
                                               : RerouteStatus_UmlNeu;
        new_events.push_back(CreateReroutedEvent(ctx.b, additional, status));
      },
      "./Service/ListUml/Uml/ListZug/Zug");

  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b, MessageUnion_RerouteMessage,
      CreateRerouteMessage(ctx.b, trip_id, ctx.b.CreateVector(cancelled_events),
                           ctx.b.CreateVector(new_events))
          .Union());
}

Offset<Message> parse_conn_decision_msg(context& ctx, xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(ctx, from_e_node);
  if (from == boost::none) {
    throw std::runtime_error("bad from event in RIS conn decision");
  }
  auto from_trip_id = parse_trip_id(ctx, from_e_node);

  std::vector<Offset<ConnectionDecision>> decisions;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(ctx, to_e_node);
    if (to == boost::none) {
      continue;
    }
    auto to_trip_id = parse_trip_id(ctx, to_e_node);

    auto hold = cstr(connection_node.attribute("Status").value()) == "Gehalten";
    decisions.push_back(CreateConnectionDecision(ctx.b, to_trip_id, *to, hold));
  }

  if (decisions.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn decision");
  }

  return CreateMessage(
      ctx.b, MessageUnion_ConnectionDecisionMessage,
      CreateConnectionDecisionMessage(ctx.b, from_trip_id, *from,
                                      ctx.b.CreateVector(decisions))
          .Union());
}

Offset<Message> parse_conn_assessment_msg(context& ctx, xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(ctx, from_e_node);
  if (from == boost::none) {
    throw std::runtime_error("bad from event in RIS conn assessment");
  }
  auto from_trip_id = parse_trip_id(ctx, from_e_node);

  std::vector<Offset<ConnectionAssessment>> assessments;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(ctx, to_e_node);
    if (to == boost::none) {
      continue;
    }
    auto to_trip_id = parse_trip_id(ctx, to_e_node);

    auto a = connection_node.attribute("Bewertung").as_int();
    assessments.push_back(
        CreateConnectionAssessment(ctx.b, to_trip_id, *to, a));
  }

  if (assessments.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn assessment");
  }

  return CreateMessage(
      ctx.b, MessageUnion_ConnectionAssessmentMessage,
      CreateConnectionAssessmentMessage(ctx.b, from_trip_id, *from,
                                        ctx.b.CreateVector(assessments))
          .Union());
}

boost::optional<ris_message> parse_message(xml_node const& msg,
                                           std::time_t t_out) {
  static std::map<cstr, parser_func_t> map(
      {{"Ist", std::bind(parse_delay_msg, _1, _2, DelayType_Is)},
       {"IstProg", std::bind(parse_delay_msg, _1, _2, DelayType_Forecast)},
       {"Ausfall", std::bind(parse_cancel_msg, _1, _2)},
       {"Zusatzzug", std::bind(parse_addition_msg, _1, _2)},
       {"Umleitung", std::bind(parse_reroute_msg, _1, _2)},
       {"Anschluss", std::bind(parse_conn_decision_msg, _1, _2)},
       {"Anschlussbewertung", std::bind(parse_conn_assessment_msg, _1, _2)}});

  auto const& payload = msg.first_child();
  auto it = map.find(payload.name());

  if (it == end(map)) {
    return boost::none;
  }

  context ctx;
  ctx.b.Finish(it->second(ctx, payload));
  return {{ctx.earliest, ctx.latest, t_out, std::move(ctx.b)}};
}

std::vector<ris_message> parse_xmls(std::vector<buffer>&& strings) {
  std::vector<ris_message> parsed_messages;
  for (auto& s : strings) {
    try {
      xml_document d;
      auto r =
          d.load_buffer_inplace(reinterpret_cast<void*>(s.data()), s.size());
      if (!r) {
        LOG(error) << "bad XML: " << r.description();
        continue;
      }

      auto t_out = parse_time(child_attr(d, "Paket", "TOut").value());
      for (auto const& msg : d.select_nodes("/Paket/ListNachricht/Nachricht")) {
        if (auto parsed_message = parse_message(msg.node(), t_out)) {
          parsed_messages.emplace_back(std::move(*parsed_message));
        }
      }
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse RIS message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse RIS message";
    }
  }

  std::sort(
      begin(parsed_messages), end(parsed_messages),
      [](ris_message const& lhs, ris_message const& rhs) {
        return std::tie(lhs.timestamp, lhs.earliest, lhs.latest, *lhs.buffer_) <
               std::tie(rhs.timestamp, rhs.earliest, rhs.latest, *rhs.buffer_);
      });

  return parsed_messages;
}

}  // namespace risml
}  // ris
}  // motis
