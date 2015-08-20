#pragma once

#include <cinttypes>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/Station_generated.h"
#include "motis/schedule-format/Attribute_generated.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct shared_data {
  dependencies(std::map<int, Offset<Station>> const& stations,
               std::map<uint16_t, Offset<Attribute>> const& attributes,
               std::map<int, Offset<String>> const& bitfields,
               platform_rules const& pf_rules)
      : stations(stations),
        attributes(attributes),
        bitfields(bitfields),
        pf_rules(pf_rules) {}

  std::map<int, Offset<Station>> const& stations;
  std::map<uint16_t, Offset<Attribute>> const& attributes;
  std::map<int, Offset<String>> const& bitfields;
  platform_rules const& pf_rules;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
