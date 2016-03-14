#pragma once

#include <map>

#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/ris/detail/parse_station.h"

#include "motis/protocol/RISMessage_generated.h"

using namespace flatbuffers;
using namespace pugi;
using namespace parser;

namespace motis {
namespace ris {
namespace detail {

std::time_t inline parse_time(cstr const& raw) {
  // format YYYYMMDDhhmmssfff (fff = millis)
  if (raw.length() < 14) {
    throw std::runtime_error("bad time format (length < 14)");
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

boost::optional<EventType> inline parse_type(
    cstr const& raw,
    boost::optional<EventType> const default_value = boost::none) {
  static const std::map<cstr, EventType> map({{"Start", EventType_Departure},
                                              {"Ab", EventType_Departure},
                                              {"An", EventType_Arrival},
                                              {"Ziel", EventType_Arrival}});
  auto it = map.find(raw);
  if (it == end(map)) {
    return default_value;
  }
  return it->second;
}

xml_attribute child_attr(xml_node const& n, char const* e, char const* a) {
  return n.child(e).attribute(a);
}

std::time_t inline parse_target_time(xml_node const& node) {
  auto const& scheduled_attr = child_attr(node, "Service", "Zielzeit");
  if (scheduled_attr.empty()) {
    throw std::runtime_error("corrupt RIS message (missing target time)");
  }
  return parse_time(scheduled_attr.value());
}

template <typename F>
void inline foreach_event(
    FlatBufferBuilder& fbb, xml_node const& msg, F func,
    char const* train_selector = "./Service/ListZug/Zug") {
  for (auto const& train : msg.select_nodes(train_selector)) {
    auto const& t_node = train.node();
    auto train_index = t_node.attribute("Nr").as_uint();
    auto line_id = t_node.attribute("Linie").value();
    auto line_id_offset = fbb.CreateString(line_id);

    for (auto const& train_event : t_node.select_nodes("./ListZE/ZE")) {
      auto const& e_node = train_event.node();
      auto event_type = parse_type(e_node.attribute("Typ").value());
      if (event_type == boost::none) {
        continue;
      }

      auto station = parse_station(fbb, e_node);
      auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

      auto event = CreateEvent(fbb, station.first, station.second, train_index,
                               line_id_offset, *event_type, scheduled);
      func(event, e_node, t_node);
    }
  }
}

Offset<AdditionalEvent> parse_additional_event(FlatBufferBuilder& fbb,
                                               Offset<Event> const& event,
                                               xml_node const& e_node,
                                               xml_node const& t_node) {
  auto track_attr = child_attr(e_node, "Gleis", "Soll");
  auto track = fbb.CreateString(track_attr ? track_attr.value() : "");
  auto category = fbb.CreateString(t_node.attribute("Gattung").value());
  return CreateAdditionalEvent(fbb, event, category, track);
}

boost::optional<Offset<Event>> inline parse_standalone_event(
    FlatBufferBuilder& fbb, xml_node const& e_node) {
  auto event_type = parse_type(e_node.attribute("Typ").value());
  if (event_type == boost::none) {
    return boost::none;
  }

  auto station = parse_station(fbb, e_node);
  auto train_index = child_attr(e_node, "Zug", "Nr").as_uint();
  auto line_id = child_attr(e_node, "Zug", "Linie").value();
  auto line_id_offset = fbb.CreateString(line_id);

  auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

  return CreateEvent(fbb, station.first, station.second, train_index,
                     line_id_offset, *event_type, scheduled);
}

Offset<TripId> inline parse_trip_id(
    FlatBufferBuilder& fbb, xml_node const& msg,
    char const* service_selector = "./Service") {
  auto const& node = msg.select_node(service_selector).node();

  auto station = parse_station(fbb, node, "IdBfEvaNr", "IdBf");
  auto train_nr = node.attribute("IdZNr").as_uint();
  auto line_id = node.attribute("IdLinie").value();
  auto type = parse_type(node.attribute("IdTyp").value(), EventType_Departure);
  auto timestamp = parse_time(node.attribute("IdZeit").value());
  auto base = CreateEvent(fbb, station.first, station.second, train_nr,
                          fbb.CreateString(line_id), *type, timestamp);

  auto t_station = parse_station(fbb, node, "ZielBfEvaNr", "ZielBfCode");
  auto t_time = parse_time(node.attribute("Zielzeit").value());
  return CreateTripId(fbb, base, t_station.first, t_station.second, t_time);
}

}  // detail
}  // ris
}  // motis
