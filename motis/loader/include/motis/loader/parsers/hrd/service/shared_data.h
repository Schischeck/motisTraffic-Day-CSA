#pragma once

#include <cinttypes>
#include <string>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/categories_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct shared_data {
  shared_data() = default;
  shared_data(std::map<int, intermediate_station> stations,
              std::map<uint32_t, category> categories,
              std::map<uint16_t, std::string> attributes,
              std::map<int, bitfield> bitfields, platform_rules pf_rules,
              std::map<uint64_t, std::string> directions)
      : stations(std::move(stations)),
        categories(std::move(categories)),
        attributes(std::move(attributes)),
        bitfields(std::move(bitfields)),
        pf_rules(std::move(pf_rules)),
        directions(std::move(directions)) {}

  std::map<int, intermediate_station> stations;
  std::map<uint32_t, category> categories;
  std::map<uint16_t, std::string> attributes;
  std::map<int, bitfield> bitfields;
  platform_rules pf_rules;
  std::map<uint64_t, std::string> directions;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
