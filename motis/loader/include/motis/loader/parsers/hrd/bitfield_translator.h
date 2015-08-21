#pragma once

#include <map>

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct bitfield_translator {
  bitfield_translator(std::map<int, bitfield> const& hrd_bitfields,
                      flatbuffers::FlatBufferBuilder& builder);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      int bitfield_num);

  flatbuffers::Offset<flatbuffers::String> get_or_create_bitfield(
      bitfield const&);

  std::map<int, bitfield> const& hrd_bitfields_;
  std::map<bitfield, flatbuffers::Offset<flatbuffers::String>,
           bitset_comparator<BIT_COUNT>> fbs_bitfields_;
  flatbuffers::FlatBufferBuilder& builder_;
};

}  // hrd
}  // loader
}  // motis
