#pragma once

#include <cinttypes>
#include <map>
#include <tuple>

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

constexpr int TIME_NOT_SET = -1;
struct platform_rule {
  flatbuffers::Offset<flatbuffers::String> platform_name;
  flatbuffers::Offset<flatbuffers::String> bitfield;
  int time;
};

typedef std::tuple<int, int, uint64_t> platform_rule_key;
typedef std::map<platform_rule_key, std::vector<platform_rule>> platform_rules;

platform_rules parse_platform_rules(
    loaded_file,
    std::map<int, flatbuffers::Offset<flatbuffers::String>> const& bitfields,
    flatbuffers::FlatBufferBuilder& b);

}  // hrd
}  // loader
}  // motis
