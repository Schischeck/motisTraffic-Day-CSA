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
struct track_rule {
  flatbuffers::Offset<flatbuffers::String> track_name_;
  int bitfield_num_;
  int time_;
};

typedef std::tuple<int, int, uint64_t> track_rule_key;
typedef std::map<track_rule_key, std::vector<track_rule>> track_rules;

track_rules parse_track_rules(loaded_file const&,
                              flatbuffers::FlatBufferBuilder& b);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
