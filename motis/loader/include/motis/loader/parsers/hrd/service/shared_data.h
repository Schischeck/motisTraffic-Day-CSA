#pragma once

#include <cinttypes>
#include <string>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/schedule-format/Station_generated.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct shared_data {
  shared_data(std::map<int, flatbuffers::Offset<Station>> const& stations,
              std::map<uint16_t, std::string> const& attributes,
              std::map<int, bitfield> const& bitfields,
              platform_rules const& pf_rules)
      : stations(std::move(stations)),
        attributes(std::move(attributes)),
        bitfields(std::move(bitfields)),
        pf_rules(std::move(pf_rules)) {}

  std::map<int, flatbuffers::Offset<Station>> stations;
  std::map<uint16_t, std::string> attributes;
  std::map<int, bitfield> bitfields;
  platform_rules pf_rules;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
