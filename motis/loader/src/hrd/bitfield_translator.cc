#include "motis/loader/parsers/hrd/bitfield_translator.h"

#include "parser/util.h"

using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

bitfield_translator::bitfield_translator(
    std::map<int, bitfield> const& hrd_bitfields, FlatBufferBuilder& builder)
    : hrd_bitfields_(hrd_bitfields), builder_(builder) {
  bitfield alternating_bits;
  for (unsigned i = 0; i < alternating_bits.size(); ++i) {
    alternating_bits.set(i, (i % 2) == 0);
  }
  fbs_bitfields_.set_empty_key(alternating_bits);
}

Offset<String> bitfield_translator::get_or_create_bitfield(int bitfield_num) {
  auto lookup_it = fbs_bf_lookup_.find(bitfield_num);
  if (lookup_it != end(fbs_bf_lookup_)) {
    return lookup_it->second;
  }

  auto hrd_bitfields_it = hrd_bitfields_.find(bitfield_num);
  verify(hrd_bitfields_it != end(hrd_bitfields_),
         "bitfield with bitfield number %d not found\n", bitfield_num);
  return get_or_create_bitfield(hrd_bitfields_it->second, bitfield_num);
}

Offset<String> bitfield_translator::get_or_create_bitfield(bitfield const& b,
                                                           int index) {
  auto fbs_bitfields_it = fbs_bitfields_.find(b);
  if (fbs_bitfields_it == end(fbs_bitfields_)) {
    auto serialized = builder_.CreateString(serialize_bitset<BIT_COUNT>(b));
    std::tie(fbs_bitfields_it, std::ignore) =
        fbs_bitfields_.insert(std::make_pair(b, serialized));

    if (index != NO_INDEX) {
      fbs_bf_lookup_.insert(std::make_pair(index, serialized));
    }
  }
  return fbs_bitfields_it->second;
}

}  // hrd
}  // loader
}  // motis
