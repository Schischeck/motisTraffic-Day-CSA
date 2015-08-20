#pragma once

#include <cinttypes>
#include <string>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/Station_generated.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct shared_data {
  shared_data(
      std::map<int, flatbuffers::Offset<Station>> const& stations,
      std::map<uint16_t, std::string> const& attributes,
      std::map<int, flatbuffers::Offset<flatbuffers::String>> const& bitfields,
      platform_rules const& pf_rules)
      : stations(stations),
        attributes(attributes),
        bitfields(bitfields),
        pf_rules(pf_rules) {}

  std::map<int, flatbuffers::Offset<Station>> const& stations;
  std::map<uint16_t, std::string> const& attributes;
  std::map<int, flatbuffers::Offset<flatbuffers::String>> const& bitfields;
  platform_rules const& pf_rules;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
