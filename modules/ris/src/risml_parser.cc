#include "motis/ris/risml_parser.h"

#include <iostream>
#include <map>

#include "boost/date_time.hpp"
#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

using namespace std::placeholders;
using namespace flatbuffers;
using namespace pugi;
using namespace parser;
using namespace motis::ris;

using parsed_msg_t = std::pair<std::time_t, Offset<Message>>;
using parser_func_t =
    std::function<parsed_msg_t(FlatBufferBuilder&, xml_node const&)>;

std::time_t parse_time(cstr const& raw) {
  // format YYYYMMDDhhmmssfff (fff = millis)
  if (raw.length() < 14) {
    return 0;
  }

  std::tm time_struct;
  time_struct.tm_year = parse<int>(raw.substr(0, size(4))) - 1900;
  time_struct.tm_mon = parse<int>(raw.substr(4, size(2))) - 1;
  time_struct.tm_mday = parse<int>(raw.substr(6, size(2)));

  time_struct.tm_hour = parse<int>(raw.substr(8, size(2)));
  time_struct.tm_min = parse<int>(raw.substr(10, size(2)));
  time_struct.tm_sec = parse<int>(raw.substr(12, size(2)));
  time_struct.tm_wday = -1;
  time_struct.tm_yday = -1;
  time_struct.tm_isdst = -1;

  return std::mktime(&time_struct);
}

EventType parse_event_type(cstr const& raw) {
  static const std::map<cstr, EventType> map({{"Start", EventType_Departure},
                                              {"Ab", EventType_Departure},
                                              {"Durch", EventType_Pass},
                                              {"An", EventType_Arrival},
                                              {"Ziel", EventType_Arrival}});
  auto it = map.find(raw);
  if (it == end(map)) {
    throw std::runtime_error("corrupt RIS message (bad event type)");
  }
  return it->second;
}

xml_attribute child_attr(xml_node const& n, char const* e, char const* a) {
  return n.child(e).attribute(a);
}

std::pair<StationIdType, Offset<String>> parse_station(FlatBufferBuilder& fbb,
                                                       xml_node const& e_node) {
  std::pair<StationIdType, Offset<String>> station;
  auto const& station_node = e_node.child("Bf");
  if (!station_node) {
    station = {StationIdType_Context, 0};
  } else {
    auto const& eva_attribute = station_node.attribute("EvaNr");
    if (!eva_attribute.empty()) {
      station.first = StationIdType_EVA;
      station.second = fbb.CreateString(eva_attribute.value());
    } else {
      station.first = StationIdType_DS100;
      station.second = fbb.CreateString(station_node.attribute("Code").value());
    }
  }
  return station;
}

template <typename F>
void foreach_event(FlatBufferBuilder& fbb, xml_node const& msg, F func,
                   char const* train_selector = "./Service/ListZug/Zug") {
  for (auto const& train : msg.select_nodes(train_selector)) {
    auto const& t_node = train.node();
    auto train_index = t_node.attribute("Nr").as_int();

    for (auto const& train_event : t_node.select_nodes("./ListZE/ZE")) {
      auto const& e_node = train_event.node();
      auto station = parse_station(fbb, e_node);
      auto event_type = parse_event_type(e_node.attribute("Typ").value());
      auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

      auto event = CreateEvent(fbb, station.first, station.second, train_index,
                               event_type, scheduled);
      func(event, e_node, t_node);
    }
  }
}

std::time_t parse_target_time(xml_node const& node) {
  auto const& scheduled_attr = child_attr(node, "Service", "Zielzeit");
  if (scheduled_attr.empty()) {
    throw std::runtime_error("corrupt RIS message (missing target time)");
  }
  return parse_time(scheduled_attr.value());
}

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

Offset<AdditionalEvent> parse_additional_event(FlatBufferBuilder& fbb,
                                               Offset<Event> const& event,
                                               xml_node const& e_node,
                                               xml_node const& t_node) {
  auto track_attr = child_attr(e_node, "Gleis", "Soll");
  auto track = (track_attr) ? fbb.CreateString(track_attr.value()) : 0;
  auto category = fbb.CreateString(t_node.attribute("Gattung").value());
  return CreateAdditionalEvent(fbb, event, category, track);
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

Offset<Event> parse_standalone_event(FlatBufferBuilder& fbb,
                                     xml_node const& e_node) {
  auto station = parse_station(fbb, e_node);
  auto train_index = child_attr(e_node, "Zug", "Nr").as_int();
  auto event_type = parse_event_type(e_node.attribute("Typ").value());
  auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

  return CreateEvent(fbb, station.first, station.second, train_index,
                     event_type, scheduled);
}

parsed_msg_t parse_conn_decision_msg(FlatBufferBuilder& fbb,
                                     xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(fbb, from_e_node);
  auto target_time = parse_target_time(from_e_node);

  std::vector<Offset<ConnectionDecision>> decisions;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(fbb, to_e_node);
    auto hold = cstr(connection_node.attribute("Status").value()) == "Gehalten";
    decisions.push_back(CreateConnectionDecision(fbb, to, hold));

    auto to_target_time = parse_target_time(to_e_node);
    if (to_target_time > target_time) {
      target_time = to_target_time;
    }
  }

  return {target_time,
          CreateMessage(fbb, MessageUnion_ConnectionDecisionMessage,
                        CreateConnectionDecisionMessage(
                            fbb, from, fbb.CreateVector(decisions))
                            .Union())};
}

parsed_msg_t parse_conn_assessment_msg(FlatBufferBuilder& fbb,
                                       xml_node const& msg) {
  auto const& from_e_node = msg.child("ZE");
  auto from = parse_standalone_event(fbb, from_e_node);
  auto target_time = parse_target_time(from_e_node);

  std::vector<Offset<ConnectionAssessment>> assessments;
  for (auto&& connection : from_e_node.select_nodes("./ListAnschl/Anschl")) {
    auto const& connection_node = connection.node();
    auto const& to_e_node = connection_node.child("ZE");
    auto to = parse_standalone_event(fbb, to_e_node);
    auto assessment = connection_node.attribute("Bewertung").as_int();
    assessments.push_back(CreateConnectionAssessment(fbb, to, assessment));

    auto to_target_time = parse_target_time(to_e_node);
    if (to_target_time > target_time) {
      target_time = to_target_time;
    }
  }

  return {target_time,
          CreateMessage(fbb, MessageUnion_ConnectionAssessmentMessage,
                        CreateConnectionAssessmentMessage(
                            fbb, from, fbb.CreateVector(assessments))
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
    return {};
  }

  FlatBufferBuilder fbb;
  auto const parsed = it->second(fbb, payload);
  fbb.Finish(parsed.second);
  return {{parsed.first, t_out, std::move(fbb)}};
}

std::vector<ris_message> motis::ris::parse_xmls(std::vector<buffer>&& strings) {
  std::vector<ris_message> parsed_messages;
  for (auto& s : strings) {
    xml_document d;
    auto r = d.load_buffer_inplace(reinterpret_cast<void*>(s.data()), s.size());
    if (!r) {
      throw std::runtime_error("bad xml: " + std::string(r.description()));
    }

    auto t_out = parse_time(d.child("Paket").attribute("TOut").value());
    for (auto const& msg : d.select_nodes("/Paket/ListNachricht/Nachricht")) {
      if (auto parsed_message = parse_message(msg.node(), t_out)) {
        parsed_messages.emplace_back(std::move(*parsed_message));
      }
    }
  }
  return parsed_messages;
}
