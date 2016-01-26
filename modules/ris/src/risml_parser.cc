#include "motis/ris/risml_parser.h"

#include <map>

#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/logging.h"
#include "motis/ris/detail/ris_parser_util.h"

using namespace std::placeholders;
using namespace flatbuffers;
using namespace pugi;
using namespace parser;
using namespace motis::logging;
using namespace motis::ris::detail;

namespace motis {
namespace ris {

using parsed_msg_t = std::pair<std::time_t, Offset<Message>>;
using parser_func_t =
    std::function<parsed_msg_t(FlatBufferBuilder&, xml_node const&)>;

parsed_msg_t parse_delay_msg(FlatBufferBuilder& fbb, xml_node const& msg,
                             DelayType type) {
  std::vector<Offset<UpdatedEvent>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const&) {
    auto attr_name = (type == DelayType_Is) ? "Ist" : "Prog";
    auto updated = parse_time(child_attr(e_node, "Zeit", attr_name).value());
    events.push_back(CreateUpdatedEvent(fbb, event, updated));
  });
  return {parse_target_time(msg),
          CreateMessage(
              fbb, MessageUnion_DelayMessage,
              CreateDelayMessage(fbb, type, fbb.CreateVector(events)).Union())};
}

parsed_msg_t parse_cancel_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<Event>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event, xml_node const&,
                              xml_node const&) { events.push_back(event); });
  return {parse_target_time(msg),
          CreateMessage(
              fbb, MessageUnion_CancelMessage,
              CreateCancelMessage(fbb, fbb.CreateVector(events)).Union())};
}

parsed_msg_t parse_addition_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<AdditionalEvent>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const& t_node) {
    events.push_back(parse_additional_event(fbb, event, e_node, t_node));
  });
  return {parse_target_time(msg),
          CreateMessage(
              fbb, MessageUnion_AdditionMessage,
              CreateAdditionMessage(fbb, fbb.CreateVector(events)).Union())};
}

parsed_msg_t parse_reroute_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<Event>> cancelled_events;
  foreach_event(fbb, msg,
                [&](Offset<Event> const& event, xml_node const&,
                    xml_node const&) { cancelled_events.push_back(event); });

  std::vector<Offset<ReroutedEvent>> new_events;
  foreach_event(
      fbb, msg,
      [&](Offset<Event> const& event, xml_node const& e_node,
          xml_node const& t_node) {
        auto additional = parse_additional_event(fbb, event, e_node, t_node);
        cstr status_str = e_node.attribute("RegSta").value();
        auto status = (status_str == "Normal") ? RerouteStatus_Normal
                                               : RerouteStatus_UmlNeu;
        new_events.push_back(CreateReroutedEvent(fbb, additional, status));
      },
      "./Service/ListUml/Uml/ListZug/Zug");

  return {parse_target_time(msg),
          CreateMessage(
              fbb, MessageUnion_RerouteMessage,
              CreateRerouteMessage(fbb, fbb.CreateVector(cancelled_events),
                                   fbb.CreateVector(new_events))
                  .Union())};
}

parsed_msg_t parse_conn_decision_msg(FlatBufferBuilder& fbb,
                                     xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(fbb, from_e_node);
  if (from == boost::none) {
    throw std::runtime_error("bad from event in RIS conn decision");
  }
  auto target_time = parse_target_time(from_e_node);

  std::vector<Offset<ConnectionDecision>> decisions;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(fbb, to_e_node);
    if (to == boost::none) {
      continue;
    }

    auto hold = cstr(connection_node.attribute("Status").value()) == "Gehalten";
    decisions.push_back(CreateConnectionDecision(fbb, *to, hold));

    auto to_target_time = parse_target_time(to_e_node);
    if (to_target_time > target_time) {
      target_time = to_target_time;
    }
  }

  if (decisions.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn decision");
  }

  return {target_time,
          CreateMessage(fbb, MessageUnion_ConnectionDecisionMessage,
                        CreateConnectionDecisionMessage(
                            fbb, *from, fbb.CreateVector(decisions))
                            .Union())};
}

parsed_msg_t parse_conn_assessment_msg(FlatBufferBuilder& fbb,
                                       xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(fbb, from_e_node);
  if (from == boost::none) {
    throw std::runtime_error("bad from event in RIS conn assessment");
  }
  auto target_time = parse_target_time(from_e_node);

  std::vector<Offset<ConnectionAssessment>> assessments;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(fbb, to_e_node);
    if (to == boost::none) {
      continue;
    }

    auto assessment = connection_node.attribute("Bewertung").as_int();
    assessments.push_back(CreateConnectionAssessment(fbb, *to, assessment));

    auto to_target_time = parse_target_time(to_e_node);
    if (to_target_time > target_time) {
      target_time = to_target_time;
    }
  }

  if (assessments.empty()) {
    throw std::runtime_error("zero valid to events in RIS conn assessment");
  }

  return {target_time,
          CreateMessage(fbb, MessageUnion_ConnectionAssessmentMessage,
                        CreateConnectionAssessmentMessage(
                            fbb, *from, fbb.CreateVector(assessments))
                            .Union())};
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

  FlatBufferBuilder fbb;
  auto const parsed = it->second(fbb, payload);
  fbb.Finish(parsed.second);
  return {{parsed.first, t_out, std::move(fbb)}};
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

      auto t_out = parse_time(d.child("Paket").attribute("TOut").value());
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
              return std::tie(lhs.timestamp, lhs.scheduled, *lhs.buffer_) <
                     std::tie(rhs.timestamp, rhs.scheduled, *rhs.buffer_);
            });

  return parsed_messages;
}

}  // ris
}  // motis
