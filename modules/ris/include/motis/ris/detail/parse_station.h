#pragma once

#include <utility>
#include <string>

#include "pugixml.hpp"

#include "motis/protocol/RISMessage_generated.h"

using namespace flatbuffers;
using namespace pugi;

namespace motis {
namespace ris {
namespace detail {

std::pair<StationIdType, Offset<String>> inline parse_station(
    FlatBufferBuilder& fbb, xml_node const& station_node,
    char const* eva_attr_name, char const* ds100_attr_name) {
  std::pair<StationIdType, Offset<String>> station{StationIdType_Context, 0};
  if (station_node) {
    auto const& eva_attribute = station_node.attribute(eva_attr_name);
    if (!eva_attribute.empty()) {
      std::string eva_string(eva_attribute.value());
      if (eva_string.size() == 6) {
        eva_string.insert(0, 1, '0');
      }

      station.first = StationIdType_EVA;
      station.second = fbb.CreateString(eva_string);
    } else {
      auto const& ds100 = station_node.attribute(ds100_attr_name).value();
      station.first = StationIdType_DS100;
      station.second = fbb.CreateString(ds100);
    }
  } else {
    station.first = StationIdType_Context;
    station.second = fbb.CreateString("");
  }
  return station;
}

std::pair<StationIdType, Offset<String>> inline parse_station(
    FlatBufferBuilder& fbb, xml_node const& e_node) {
  auto const& station_node = e_node.child("Bf");
  return parse_station(fbb, station_node, "EvaNr", "Code");
}

}  // detail
}  // namespace ris
}  // namespace motis
