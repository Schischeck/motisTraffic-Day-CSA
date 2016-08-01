#pragma once

#include <map>

#include "flatbuffers/flatbuffers.h"

#include "google/dense_hash_map"

#include "motis/loader/bitfield.h"

namespace motis {
namespace loader {
namespace hrd {

struct bitfield_builder {
  static constexpr int no_bitfield_num_ = -1;

  explicit bitfield_builder(std::map<int, bitfield>);

  flatbuffers64::Offset<flatbuffers64::String> get_or_create_bitfield(
      int bitfield_num, flatbuffers64::FlatBufferBuilder&);

  flatbuffers64::Offset<flatbuffers64::String> get_or_create_bitfield(
      bitfield const&, flatbuffers64::FlatBufferBuilder&,
      int = no_bitfield_num_);

  std::map<int, bitfield> const hrd_bitfields_;
  google::dense_hash_map<bitfield, flatbuffers64::Offset<flatbuffers64::String>,
                         std::hash<bitfield>, std::equal_to<bitfield>>
      fbs_bitfields_;
  std::map<int, flatbuffers64::Offset<flatbuffers64::String>> fbs_bf_lookup_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
