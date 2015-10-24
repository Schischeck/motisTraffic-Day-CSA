#pragma once

#include <map>

#include "flatbuffers/flatbuffers.h"

#include "google/dense_hash_map"

#include "motis/loader/bitfield.h"

namespace motis {
namespace loader {
namespace hrd {

struct bitfield_builder {
  static constexpr int NO_BITFIELD_NUM = -1;

  bitfield_builder(std::map<int, bitfield>);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      int bitfield_num, flatbuffers::FlatBufferBuilder&);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      bitfield const&, int bitfield_num = NO_BITFIELD_NUM,
      flatbuffers::FlatBufferBuilder&);

  std::map<int, bitfield> const hrd_bitfields_;
  google::dense_hash_map<bitfield, flatbuffers::Offset<flatbuffers::String>,
                         std::hash<bitfield>,
                         std::equal_to<bitfield>> fbs_bitfields_;
  std::map<int, flatbuffers::Offset<flatbuffers::String>> fbs_bf_lookup_;
};

}  // hrd
}  // loader
}  // motis
