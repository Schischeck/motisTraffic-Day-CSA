#pragma once

#include <functional>
#include <map>

#include "google/dense_hash_map"

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct bitfield_translator {
  static constexpr int NO_INDEX = -1;

  bitfield_translator(std::map<int, bitfield> const& hrd_bitfields,
                      flatbuffers::FlatBufferBuilder& builder);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      int bitfield_num);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      bitfield const&, int index = NO_INDEX);

  std::map<int, bitfield> const& hrd_bitfields_;
  google::dense_hash_map<bitfield, flatbuffers::Offset<flatbuffers::String>,
                         std::hash<bitfield>,
                         std::equal_to<bitfield>> fbs_bitfields_;
  std::map<int, flatbuffers::Offset<flatbuffers::String>> fbs_bf_lookup_;
  flatbuffers::FlatBufferBuilder& builder_;
};

}  // hrd
}  // loader
}  // motis
