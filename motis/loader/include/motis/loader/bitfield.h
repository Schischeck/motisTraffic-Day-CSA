#pragma once

#include <cassert>
#include <bitset>
#include <algorithm>

namespace motis {
namespace loader {

constexpr int BIT_COUNT = 512;
typedef std::bitset<BIT_COUNT> bitfield;

template <std::size_t BitSetSize>
struct bitset_comparator {
  bool operator()(std::bitset<BitSetSize> const& lhs,
                  std::bitset<BitSetSize> const& rhs) const {
    for (std::size_t i = 0; i < BitSetSize; ++i) {
      int lhs_bit = lhs.test(i) ? 1 : 0;
      int rhs_bit = rhs.test(i) ? 1 : 0;
      if (lhs_bit != rhs_bit) {
        return lhs_bit < rhs_bit;
      }
    }
    return false;
  }
};

template <std::size_t BitSetSize>
std::bitset<BitSetSize> create_uniform_bitfield(char val) {
  assert(val == '1' || val == '0');

  std::string all_days_bit_str;
  all_days_bit_str.resize(BitSetSize);
  std::fill(begin(all_days_bit_str), end(all_days_bit_str), val);

  return std::bitset<BitSetSize>(all_days_bit_str);
}

template <int BitCount>
inline std::string serialize_bitset(std::bitset<BitCount> const& bitset) {
  constexpr int number_of_bytes = BitCount >> 3;

  char buf[number_of_bytes];
  std::fill(std::begin(buf), std::end(buf), 0);

  int bit_it = 0;
  for (int byte = 0; byte < number_of_bytes; ++byte) {
    uint8_t next_byte = 0;
    for (int bit = 0; bit < 8; ++bit) {
      uint8_t bit_value = bitset.test(bit_it) ? 1 : 0;
      next_byte += (bit_value << bit);
      ++bit_it;
    }
    buf[byte] = next_byte;
  }

  return std::string(buf, number_of_bytes);
}

template<typename IntType>
inline bool is_bit_set(int bit_idx, IntType value) {
  return (value >> bit_idx) & 1;
}

template <int BitCount>
inline std::bitset<BitCount> deserialize_bitset(parser::cstr str) {
  constexpr std::size_t number_of_bytes = BitCount >> 3;

  std::bitset<BitCount> bits;
  int bit_it = 0;
  auto limit = std::min(number_of_bytes, str.len);
  for (unsigned byte = 0; byte < limit; ++byte) {
    for (int i = 0; i < 8; ++i) {
      bits.set(bit_it, is_bit_set(i, str[byte]));
      ++bit_it;
    }
  }

  return bits;
}

}  // loader
}  // motis
