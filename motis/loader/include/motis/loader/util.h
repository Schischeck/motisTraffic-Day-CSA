#pragma once

#include <cmath>
#include <cinttypes>
#include <string>
#include <bitset>
#include <algorithm>
#include <map>

#include "parser/cstr.h"

#include "boost/filesystem/path.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {

void write_schedule(flatbuffers::FlatBufferBuilder& b,
                    boost::filesystem::path const& path);

inline flatbuffers::Offset<flatbuffers::String> to_fbs_string(
    flatbuffers::FlatBufferBuilder& b, parser::cstr s) {
  return b.CreateString(s.str, s.len);
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

template <int BitCount>
inline std::bitset<BitCount> deserialize_bitset(parser::cstr str) {
  constexpr std::size_t number_of_bytes = BitCount >> 3;

  std::bitset<BitCount> bits;
  int bit_it = 0;
  auto limit = std::min(number_of_bytes, str.len);
  for (unsigned byte = 0; byte < limit; ++byte) {
    std::bitset<8> byte_bit_set(str[byte]);
    for (int i = 0; i < 8; ++i) {
      bits.set(bit_it, byte_bit_set.test(i));
      ++bit_it;
    }
  }

  return bits;
}

template <typename IndexType, typename ValueType>
inline std::vector<ValueType> values(std::map<IndexType, ValueType> const& m) {
  std::vector<ValueType> v(m.size());
  std::transform(begin(m), end(m), begin(v),
                 [](std::pair<IndexType, ValueType> const& entry) {
    return entry.second;
  });
  return v;
}

template <typename IntType>
inline IntType raw_to_int(parser::cstr s) {
  IntType key = 0;
  std::memcpy(&key, s.str, std::min(s.len, sizeof(IntType)));
  return key;
}

inline int hhmm_to_min(int hhmm) {
  if (hhmm < 0) {
    return hhmm;
  } else {
    return (hhmm / 100) * 60 + (hhmm % 100);
  }
}

template <typename It, typename Predicate>
inline It find_nth(It begin, It end, std::size_t n, Predicate fun) {
  assert(n != 0);
  std::size_t num_elements_found = 0;
  auto it = begin;
  while (it != end && num_elements_found != n) {
    it = std::find_if(it, end, fun);
    ++num_elements_found;
    if (num_elements_found != n) {
      ++it;
    }
  }
  return it;
}

template <typename TargetCollection, typename It, typename UnaryOperation>
inline TargetCollection transform(It begin, It end, UnaryOperation op) {
  TargetCollection c;
  std::transform(begin, end, std::back_insert_iterator<TargetCollection>(c),
                 op);
  return c;
}

}  // namespace loader
}  // namespace motis
