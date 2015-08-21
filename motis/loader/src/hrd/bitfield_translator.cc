#include "motis/loader/parsers/hrd/bitfield_translator.h"

#include "parser/util.h"

using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

bitfield_translator::bitfield_translator(
    std::map<int, bitfield> const& hrd_bitfields, FlatBufferBuilder& builder)
    : hrd_bitfields_(hrd_bitfields), builder_(builder) {}

Offset<String> bitfield_translator::get_or_create_bitfield(int bitfield_num) {
  auto hrd_bitfields_it = hrd_bitfields_.find(bitfield_num);
  verify(hrd_bitfields_it != end(hrd_bitfields_),
         "bitfield with bitfield number %d not found\n", bitfield_num);
  return get_or_create_bitfield(hrd_bitfields_it->second);
}

Offset<String> bitfield_translator::get_or_create_bitfield(bitfield const& b) {
  auto fbs_bitfields_it = fbs_bitfields_.find(b);
  if (fbs_bitfields_it == end(fbs_bitfields_)) {
    std::tie(fbs_bitfields_it, std::ignore) = fbs_bitfields_.emplace(
        b, builder_.CreateString(serialize_bitset<BIT_COUNT>(b)));
  }
  return fbs_bitfields_it->second;
}

}  // hrd
}  // loader
}  // motis
