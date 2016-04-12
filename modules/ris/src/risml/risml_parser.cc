#include "motis/ris/risml/risml_parser.h"

#include <map>

#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/logging.h"
#include "motis/ris/risml/common.h"
#include "motis/ris/risml/parse_event.h"
#include "motis/ris/risml/parse_station.h"
#include "motis/ris/risml/parse_time.h"
#include "motis/ris/risml/parse_type.h"

using namespace std::placeholders;
using namespace flatbuffers;
using namespace pugi;
using namespace parser;
using namespace motis::logging;

namespace motis {
namespace ris {
namespace risml {

template <typename F>
void inline foreach_event(
    context& ctx, xml_node const& msg, F func,
    char const* train_selector = "./Service/ListZug/Zug") {
  for (auto const& train : msg.select_nodes(train_selector)) {
    auto const& t_node = train.node();
    auto service_num = t_node.attribute("Nr").as_uint();
    auto line_id = t_node.attribute("Linie").value();
    auto line_id_offset = ctx.b_.CreateString(line_id);

    for (auto const& train_event : t_node.select_nodes("./ListZE/ZE")) {
      auto const& e_node = train_event.node();
      auto event_type = parse_type(e_node.attribute("Typ").value());
      if (event_type == boost::none) {
        continue;
      }

      auto station_id = parse_station(ctx.b_, e_node);
      auto schedule_time =
          parse_schedule_time(ctx, child_attr(e_node, "Zeit", "Soll").value());

      auto event = CreateEvent(ctx.b_, station_id, service_num, line_id_offset,
                               *event_type, schedule_time);
      func(event, e_node, t_node);
    }
  }
}

Offset<TripId> inline parse_trip_id(
    context& ctx, xml_node const& msg,
    char const* service_selector = "./Service") {
  auto const& node = msg.select_node(service_selector).node();

  auto station_id = parse_station(ctx.b_, node, "IdBfEvaNr");
  auto service_num = node.attribute("IdZNr").as_uint();
  auto schedule_time =
      parse_schedule_time(ctx, node.attribute("IdZeit").value());

  std::string reg_sta(node.attribute("RegSta").value());
  auto trip_type = (reg_sta == "" || reg_sta == "Plan") ? TripType_Schedule
                                                        : TripType_Additional;
  // desired side-effect: update temporal bounds
  parse_schedule_time(ctx, node.attribute("Zielzeit").value());

  return CreateTripId(ctx.b_, station_id, service_num, schedule_time,
                      trip_type);
}

using parser_func_t = std::function<Offset<Message>(context&, xml_node const&)>;

Offset<Message> parse_delay_msg(context& ctx, xml_node const& msg,
                                DelayType type) {
  std::vector<Offset<UpdatedEvent>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const&) {
    auto attr_name = (type == DelayType_Is) ? "Ist" : "Prog";
    auto updated = parse_time(child_attr(e_node, "Zeit", attr_name).value());
    events.push_back(CreateUpdatedEvent(ctx.b_, event, updated));
  });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b_, MessageUnion_DelayMessage,
      CreateDelayMessage(ctx.b_, trip_id, type, ctx.b_.CreateVector(events))
          .Union());
}

Offset<Message> parse_cancel_msg(context& ctx, xml_node const& msg) {
  std::vector<Offset<Event>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event, xml_node const&,
                              xml_node const&) { events.push_back(event); });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b_, MessageUnion_CancelMessage,
      CreateCancelMessage(ctx.b_, trip_id, ctx.b_.CreateVector(events))
          .Union());
}

Offset<Message> parse_addition_msg(context& ctx, xml_node const& msg) {
  std::vector<Offset<AdditionalEvent>> events;
  foreach_event(ctx, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const& t_node) {
    events.push_back(parse_additional_event(ctx.b_, event, e_node, t_node));
  });
  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b_, MessageUnion_AdditionMessage,
      CreateAdditionMessage(ctx.b_, trip_id, ctx.b_.CreateVector(events))
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
        auto additional = parse_additional_event(ctx.b_, event, e_node, t_node);
        cstr status_str = e_node.attribute("RegSta").value();
        auto status = (status_str == "Normal") ? RerouteStatus_Normal
                                               : RerouteStatus_UmlNeu;
        new_events.push_back(CreateReroutedEvent(ctx.b_, additional, status));
      },
      "./Service/ListUml/Uml/ListZug/Zug");

  auto trip_id = parse_trip_id(ctx, msg);
  return CreateMessage(
      ctx.b_, MessageUnion_RerouteMessage,
      CreateRerouteMessage(ctx.b_, trip_id,
                           ctx.b_.CreateVector(cancelled_events),
                           ctx.b_.CreateVector(new_events))
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
    decisions.push_back(
        CreateConnectionDecision(ctx.b_, to_trip_id, *to, hold));
  }

  if (decisions.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn decision");
  }

  return CreateMessage(
      ctx.b_, MessageUnion_ConnectionDecisionMessage,
      CreateConnectionDecisionMessage(ctx.b_, from_trip_id, *from,
                                      ctx.b_.CreateVector(decisions))
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
        CreateConnectionAssessment(ctx.b_, to_trip_id, *to, a));
  }

  if (assessments.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn assessment");
  }

  return CreateMessage(
      ctx.b_, MessageUnion_ConnectionAssessmentMessage,
      CreateConnectionAssessmentMessage(ctx.b_, from_trip_id, *from,
                                        ctx.b_.CreateVector(assessments))
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
  ctx.b_.Finish(it->second(ctx, payload));
  return {{ctx.earliest_, ctx.latest_, t_out, std::move(ctx.b_)}};
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

  std::sort(begin(parsed_messages), end(parsed_messages),
            [](ris_message const& lhs, ris_message const& rhs) {
              return std::tie(lhs.timestamp_, lhs.earliest_, lhs.latest_,
                              *lhs.buffer_) <
                     std::tie(rhs.timestamp_, rhs.earliest_, rhs.latest_,
                              *rhs.buffer_);
            });

  return parsed_messages;
}

}  // namespace risml
}  // namespace ris
}  // namespace motis
