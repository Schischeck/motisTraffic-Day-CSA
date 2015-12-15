#pragma once

#include <map>

#include "boost/optional.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

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

boost::optional<EventType> inline parse_event_type(cstr const& raw) {
  static const std::map<cstr, EventType> map({{"Start", EventType_Departure},
                                              {"Ab", EventType_Departure},
                                              {"An", EventType_Arrival},
                                              {"Ziel", EventType_Arrival}});
  auto it = map.find(raw);
  if (it == end(map)) {
    return boost::none;
  }
  return it->second;
}

xml_attribute child_attr(xml_node const& n, char const* e, char const* a) {
  return n.child(e).attribute(a);
}

std::pair<StationIdType, Offset<String>> inline parse_station(
    FlatBufferBuilder& fbb, xml_node const& e_node) {
  std::pair<StationIdType, Offset<String>> station{StationIdType_Context, 0};
  auto const& station_node = e_node.child("Bf");
  if (station_node) {
    auto const& eva_attribute = station_node.attribute("EvaNr");
    if (!eva_attribute.empty()) {
      std::string eva_string(eva_attribute.value());
      if(eva_string.size() == 6) {
        eva_string.insert(0, 1, '0');
      }

      station.first = StationIdType_EVA;
      station.second = fbb.CreateString(eva_string);
    } else {
      station.first = StationIdType_DS100;
      station.second = fbb.CreateString(station_node.attribute("Code").value());
    }
  } else {
    station.second = fbb.CreateString("");
  }
  return station;
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

    for (auto const& train_event : t_node.select_nodes("./ListZE/ZE")) {
      auto const& e_node = train_event.node();
      auto event_type = parse_event_type(e_node.attribute("Typ").value());
      if (event_type == boost::none) {
        continue;
      }

      auto station = parse_station(fbb, e_node);
      auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

      auto event = CreateEvent(fbb, station.first, station.second, train_index,
                               *event_type, scheduled);
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
  auto event_type = parse_event_type(e_node.attribute("Typ").value());
  if (event_type == boost::none) {
    return boost::none;
  }

  auto station = parse_station(fbb, e_node);
  auto train_index = child_attr(e_node, "Zug", "Nr").as_uint();
  auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

  return CreateEvent(fbb, station.first, station.second, train_index,
                     *event_type, scheduled);
}

}  // detail
}  // ris
}  // motis
